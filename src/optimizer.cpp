#include "optimizer.h"

#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace {

IROperand makeIntConstant(int value) {
    IROperand operand{};
    operand.kind = IROperandKind::IntConstant;
    operand.intValue = value;
    return operand;
}

bool isIntConstantOf(const IROperand& operand, int value) {
    return operand.kind == IROperandKind::IntConstant && operand.intValue == value;
}

bool operandsEqual(const IROperand& a, const IROperand& b) {
    if (a.kind != b.kind) {
        return false;
    }
    if (a.kind == IROperandKind::IntConstant) {
        return a.intValue == b.intValue;
    }
    return a.text == b.text;
}

// After a loop or if/else, we cannot assume that values changed inside
// it still have their old values. We therefore keep a constant only
// when its value is unchanged from before the region; otherwise we
// forget it (this also correctly drops anything newly introduced inside
// the region, since it never appears in "before").
std::unordered_map<std::string, IROperand> keepUnchanged(
    const std::unordered_map<std::string, IROperand>& before,
    const std::unordered_map<std::string, IROperand>& after) {
    std::unordered_map<std::string, IROperand> result;
    for (const auto& entry : before) {
        auto it = after.find(entry.first);
        if (it != after.end() && operandsEqual(it->second, entry.second)) {
            result.insert(entry);
        }
    }
    return result;
}

}  // namespace

IRProgram Optimizer::optimize(const IRProgram& input) {
    stats = OptimizationStats{};
    stats.originalInstructionCount = static_cast<int>(input.size());

    IRProgram folded = foldAndPropagate(input);
    IRProgram cleaned = eliminateDeadTemporaries(folded);

    stats.optimizedInstructionCount = static_cast<int>(cleaned.size());
    stats.instructionsRemoved = stats.originalInstructionCount - stats.optimizedInstructionCount;

    return cleaned;
}

OptimizationStats Optimizer::getStats() const {
    return stats;
}

IRProgram Optimizer::foldAndPropagate(const IRProgram& input) {
    IRProgram output;
    ConstantMap knownConstants;

    // Snapshots of knownConstants taken at each LOOP_START, so the
    // matching LOOP_END can restore exactly what was known before the
    // loop body started (see the "barrier" note in optimizer.h).
    std::vector<ConstantMap> savedStates;

    for (const IRInstruction& original : input) {
        IRInstruction instruction = original;

        if (instruction.opcode == IROpcode::LOOP_START) {
            instruction.operandA = resolveOperand(instruction.operandA, knownConstants);
            savedStates.push_back(knownConstants);
            output.push_back(instruction);
            continue;
        }

        if (instruction.opcode == IROpcode::LOOP_END) {
            output.push_back(instruction);
            if (!savedStates.empty()) {
                knownConstants = keepUnchanged(savedStates.back(), knownConstants);
                savedStates.pop_back();
            }
            continue;
        }

        if (instruction.opcode == IROpcode::PRINT) {
            // printf's arguments live in operandList, not operandA/operandB -
            // resolve each one against known constants in place.
            for (IROperand& argument : instruction.operandList) {
                argument = resolveOperand(argument, knownConstants);
            }
            output.push_back(instruction);
            continue;
        }

        if (instruction.opcode == IROpcode::STORE) {
            instruction.operandA = resolveOperand(instruction.operandA, knownConstants);
            output.push_back(instruction);
            recordIfConstant(instruction.destination, instruction.operandA, knownConstants);
            continue;
        }

        if (instruction.opcode == IROpcode::RETURN) {
            if (instruction.hasOperand) {
                instruction.operandA = resolveOperand(instruction.operandA, knownConstants);
            }
            output.push_back(instruction);
            continue;
        }

        if (instruction.opcode == IROpcode::IF_START) {
            instruction.operandA = resolveOperand(instruction.operandA, knownConstants);
            savedStates.push_back(knownConstants);
            output.push_back(instruction);
            continue;
        }

        if (instruction.opcode == IROpcode::ELSE) {
            output.push_back(instruction);
            if (!savedStates.empty()) {
                // Restore without popping - IF_END needs this same
                // snapshot again once the else-branch is done.
                knownConstants = savedStates.back();
            }
            continue;
        }

        if (instruction.opcode == IROpcode::IF_END) {
            output.push_back(instruction);
            if (!savedStates.empty()) {
                knownConstants = keepUnchanged(savedStates.back(), knownConstants);
                savedStates.pop_back();
            }
            continue;
        }

        if (instruction.opcode == IROpcode::COMPARE) {
            IROperand left = resolveOperand(instruction.operandA, knownConstants);
            IROperand right = resolveOperand(instruction.operandB, knownConstants);

            if (left.kind == IROperandKind::IntConstant &&
                right.kind == IROperandKind::IntConstant) {
                int folded = foldComparison(instruction.comparisonOp, left.intValue, right.intValue);
                IRInstruction store{};
                store.opcode = IROpcode::STORE;
                store.destination = instruction.destination;
                store.operandA = makeIntConstant(folded);
                store.line = instruction.line;
                output.push_back(store);
                recordIfConstant(store.destination, store.operandA, knownConstants);
                stats.constantFoldingCount++;
                continue;
            }

            instruction.operandA = left;
            instruction.operandB = right;
            output.push_back(instruction);
            knownConstants.erase(instruction.destination);
            continue;
        }

        // IROpcode::BINARY_OP
        IROperand left = resolveOperand(instruction.operandA, knownConstants);
        IROperand right = resolveOperand(instruction.operandB, knownConstants);

        if (left.kind == IROperandKind::IntConstant && right.kind == IROperandKind::IntConstant) {
            std::optional<int> folded =
                foldArithmetic(instruction.operatorSymbol, left.intValue, right.intValue);
            if (folded.has_value()) {
                IRInstruction store{};
                store.opcode = IROpcode::STORE;
                store.destination = instruction.destination;
                store.operandA = makeIntConstant(*folded);
                store.line = instruction.line;
                output.push_back(store);
                recordIfConstant(store.destination, store.operandA, knownConstants);
                stats.constantFoldingCount++;
                continue;
            }
            // Division by zero: fall through and keep the instruction as
            // a BINARY_OP instead of computing a bad result.
        }

        std::optional<IROperand> simplified =
            simplifyIdentity(instruction.operatorSymbol, left, right);
        if (simplified.has_value()) {
            IRInstruction store{};
            store.opcode = IROpcode::STORE;
            store.destination = instruction.destination;
            store.operandA = *simplified;
            store.line = instruction.line;
            output.push_back(store);
            recordIfConstant(store.destination, store.operandA, knownConstants);
            stats.algebraicSimplificationCount++;
            continue;
        }

        instruction.operandA = left;
        instruction.operandB = right;
        output.push_back(instruction);
        knownConstants.erase(instruction.destination);
    }

    return output;
}

IRProgram Optimizer::eliminateDeadTemporaries(const IRProgram& input) {
    std::unordered_set<std::string> usedNames;

    for (const IRInstruction& instruction : input) {
        if (instruction.operandA.kind == IROperandKind::Name) {
            usedNames.insert(instruction.operandA.text);
        }
        if (instruction.operandB.kind == IROperandKind::Name) {
            usedNames.insert(instruction.operandB.text);
        }
        for (const IROperand& argument : instruction.operandList) {
            if (argument.kind == IROperandKind::Name) {
                usedNames.insert(argument.text);
            }
        }
    }

    IRProgram output;
    for (const IRInstruction& instruction : input) {
        bool hasDestination = instruction.opcode == IROpcode::STORE ||
                              instruction.opcode == IROpcode::BINARY_OP ||
                              instruction.opcode == IROpcode::COMPARE;
        bool isTemp = instruction.destination.rfind("$t", 0) == 0;
        bool isUnused = usedNames.find(instruction.destination) == usedNames.end();

        if (hasDestination && isTemp && isUnused) {
            stats.deadTemporaryEliminationCount++;
            continue;  // dead compiler-generated temporary - drop it
        }

        output.push_back(instruction);
    }

    return output;
}

IROperand Optimizer::resolveOperand(const IROperand& operand, const ConstantMap& knownConstants) {
    if (operand.kind != IROperandKind::Name) {
        return operand;
    }

    auto it = knownConstants.find(operand.text);
    if (it == knownConstants.end()) {
        return operand;
    }

    stats.constantPropagationCount++;
    return it->second;
}

void Optimizer::recordIfConstant(const std::string& destination, const IROperand& value,
                                  ConstantMap& knownConstants) const {
    if (value.kind == IROperandKind::IntConstant || value.kind == IROperandKind::StringConstant) {
        knownConstants[destination] = value;
    } else {
        knownConstants.erase(destination);
    }
}

std::optional<int> Optimizer::foldArithmetic(char operatorSymbol, int left, int right) const {
    switch (operatorSymbol) {
        case '+':
            return left + right;
        case '-':
            return left - right;
        case '*':
            return left * right;
        case '/':
            if (right == 0) {
                return std::nullopt;
            }
            return left / right;
        default:
            return std::nullopt;
    }
}

int Optimizer::foldComparison(ComparisonOp op, int left, int right) const {
    switch (op) {
        case ComparisonOp::Equal:
            return left == right ? 1 : 0;
        case ComparisonOp::NotEqual:
            return left != right ? 1 : 0;
        case ComparisonOp::Less:
            return left < right ? 1 : 0;
        case ComparisonOp::LessEqual:
            return left <= right ? 1 : 0;
        case ComparisonOp::Greater:
            return left > right ? 1 : 0;
        case ComparisonOp::GreaterEqual:
            return left >= right ? 1 : 0;
        default:
            throw std::runtime_error("Internal error: unrecognized comparison operator.");
    }
}

std::optional<IROperand> Optimizer::simplifyIdentity(char operatorSymbol, const IROperand& left,
                                                      const IROperand& right) const {
    if (operatorSymbol == '+') {
        if (isIntConstantOf(left, 0)) {
            return right;  // 0 + x -> x
        }
        if (isIntConstantOf(right, 0)) {
            return left;  // x + 0 -> x
        }
    } else if (operatorSymbol == '-') {
        if (isIntConstantOf(right, 0)) {
            return left;  // x - 0 -> x
        }
    } else if (operatorSymbol == '*') {
        if (isIntConstantOf(left, 1)) {
            return right;  // 1 * x -> x
        }
        if (isIntConstantOf(right, 1)) {
            return left;  // x * 1 -> x
        }
        if (isIntConstantOf(left, 0) || isIntConstantOf(right, 0)) {
            return makeIntConstant(0);  // x * 0 -> 0, 0 * x -> 0
        }
    }

    return std::nullopt;
}
