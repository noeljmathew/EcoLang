#include "energy.h"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

std::string timeComplexityFor(int maxNestingDepth) {
    switch (maxNestingDepth) {
        case 0:
            return "O(1)";
        case 1:
            return "O(n)";
        case 2:
            return "O(n^2)";
        case 3:
            return "O(n^3)";
        default:
            return "O(n^" + std::to_string(maxNestingDepth) + ")";
    }
}

void addScaled(EnergyBreakdown& total, const EnergyBreakdown& part, long long multiplier) {
    total.assignmentCount += part.assignmentCount * multiplier;
    total.arithmeticCount += part.arithmeticCount * multiplier;
    total.comparisonCount += part.comparisonCount * multiplier;
    total.loopIterationCount += part.loopIterationCount * multiplier;
    total.printCount += part.printCount * multiplier;
}

void multiplyInPlace(EnergyBreakdown& breakdown, long long factor) {
    breakdown.assignmentCount *= factor;
    breakdown.arithmeticCount *= factor;
    breakdown.comparisonCount *= factor;
    breakdown.loopIterationCount *= factor;
    breakdown.printCount *= factor;
}

// Splits the program into each function's own body (the instructions
// strictly between its FUNC_START and FUNC_END), keyed by function name.
// Functions never nest, so this only needs a single flag, not a stack.
std::unordered_map<std::string, IRProgram> splitIntoFunctionBodies(const IRProgram& program) {
    std::unordered_map<std::string, IRProgram> bodies;
    std::string currentName;
    bool insideFunction = false;

    for (const IRInstruction& instruction : program) {
        if (instruction.opcode == IROpcode::FUNC_START) {
            currentName = instruction.functionName;
            insideFunction = true;
            bodies[currentName] = IRProgram{};
            continue;
        }
        if (instruction.opcode == IROpcode::FUNC_END) {
            insideFunction = false;
            continue;
        }
        if (insideFunction) {
            bodies[currentName].push_back(instruction);
        }
    }

    return bodies;
}

struct FunctionCost {
    EnergyBreakdown breakdown;
    int maxNestingDepth = 0;
};

// Shared across the whole recursive computation, starting from main.
struct FunctionCostContext {
    const std::unordered_map<std::string, IRProgram>& bodies;
    std::unordered_map<std::string, FunctionCost> memo;
    std::unordered_set<std::string> inProgress;
};

FunctionCost computeFunctionCost(const std::string& name, FunctionCostContext& context);

// One function's own linear scan - the same switch the whole program used
// to get in one piece, now scoped to a single function's body. Loop
// nesting (multiplier, maxNestingDepth) is purely local: a function is
// scanned once, "as if called a single time", and CALL sites are scaled
// by the caller afterward.
FunctionCost scanFunctionBody(const std::string& name, FunctionCostContext& context,
                              bool& recursionDetected) {
    FunctionCost result;
    std::vector<long long> multiplierStack;
    long long multiplier = 1;
    int nestingDepth = 0;

    for (const IRInstruction& instruction : context.bodies.at(name)) {
        switch (instruction.opcode) {
            case IROpcode::STORE:
                result.breakdown.assignmentCount += multiplier;
                break;

            case IROpcode::BINARY_OP:
                result.breakdown.arithmeticCount += multiplier;
                break;

            case IROpcode::PRINT:
                result.breakdown.printCount += multiplier;
                break;

            case IROpcode::COMPARE:
                result.breakdown.comparisonCount += multiplier;
                break;

            case IROpcode::RETURN:
            case IROpcode::IF_START:
            case IROpcode::ELSE:
            case IROpcode::IF_END:
            case IROpcode::FUNC_START:
            case IROpcode::FUNC_END:
                // Structural markers only (FUNC_START/FUNC_END never
                // appear inside a single function's own body span - they
                // were already stripped out by splitIntoFunctionBodies).
                break;

            case IROpcode::LOOP_START:
                multiplierStack.push_back(multiplier);
                multiplier *= instruction.operandA.intValue;
                result.breakdown.loopIterationCount += multiplier;

                nestingDepth++;
                if (nestingDepth > result.maxNestingDepth) {
                    result.maxNestingDepth = nestingDepth;
                }
                break;

            case IROpcode::LOOP_END:
                if (multiplierStack.empty()) {
                    throw std::runtime_error("Energy analysis error on line " +
                                             std::to_string(instruction.line) +
                                             ": unmatched LOOP_END (no matching LOOP_START).");
                }
                multiplier = multiplierStack.back();
                multiplierStack.pop_back();
                nestingDepth--;
                break;

            case IROpcode::CALL: {
                const std::string& callee = instruction.functionName;
                if (context.inProgress.count(callee) > 0) {
                    // Calling back into a function still being computed
                    // (direct or mutual recursion) - do not try to expand
                    // it further here. See the recursion-depth multiplier
                    // applied below, once this function's own scan is done.
                    recursionDetected = true;
                    break;
                }
                FunctionCost calleeCost = computeFunctionCost(callee, context);
                addScaled(result.breakdown, calleeCost.breakdown, multiplier);
                break;
            }
        }
    }

    return result;
}

// Determinism note: this always starts from "main" and walks the call
// graph depth-first, visiting each function's calls in the fixed order
// they appear in its own body (never reordered, never dependent on any
// container's iteration order - inProgress/memo are only ever looked up
// by exact name, never iterated). So for a given program, exactly the
// same function's scan will always be the one that detects a given
// recursive cycle, every time: whichever function's own body literally
// contains the call back to an already-in-progress ancestor is the one
// whose tally gets multiplied by kAssumedRecursionDepth. For mutual
// recursion (A calls B calls A), that is B in this example, not A - a
// known, documented asymmetry of this simplified model, not an attempt
// at symmetric graph analysis.
FunctionCost computeFunctionCost(const std::string& name, FunctionCostContext& context) {
    auto memoIt = context.memo.find(name);
    if (memoIt != context.memo.end()) {
        return memoIt->second;
    }

    if (context.bodies.find(name) == context.bodies.end()) {
        throw std::runtime_error("Energy analysis error: no function body found for '" + name +
                                 "'.");
    }

    context.inProgress.insert(name);
    bool recursionDetected = false;
    FunctionCost cost = scanFunctionBody(name, context, recursionDetected);
    context.inProgress.erase(name);

    if (recursionDetected) {
        multiplyInPlace(cost.breakdown, kAssumedRecursionDepth);
    }

    context.memo[name] = cost;
    return cost;
}

}  // namespace

EnergyReport analyzeEnergy(const IRProgram& program) {
    std::unordered_map<std::string, IRProgram> bodies = splitIntoFunctionBodies(program);
    FunctionCostContext context{bodies, {}, {}};

    // main() is the program's only real entry point (the synthetic final
    // CALL main is what actually "runs" the program) - a function defined
    // but never reachable from main simply never has computeFunctionCost
    // called on it, correctly contributing nothing.
    FunctionCost mainCost = computeFunctionCost("main", context);

    EnergyReport report;
    EnergyBreakdown& breakdown = report.breakdown;
    breakdown = mainCost.breakdown;

    report.totalOperations = breakdown.assignmentCount + breakdown.arithmeticCount +
                              breakdown.comparisonCount + breakdown.loopIterationCount +
                              breakdown.printCount;

    // Energy breakdown: count x cost per category. totalEnergyUnits is
    // simply their sum, so there is one single source of truth for it.
    report.assignmentEnergy = breakdown.assignmentCount * kAssignmentCost;
    report.arithmeticEnergy = breakdown.arithmeticCount * kArithmeticCost;
    report.comparisonEnergy = breakdown.comparisonCount * kComparisonCost;
    report.loopEnergy = breakdown.loopIterationCount * kLoopOverheadUnitsPerIteration;
    report.printEnergy = breakdown.printCount * kPrintCost;

    report.totalEnergyUnits = report.assignmentEnergy + report.arithmeticEnergy +
                               report.comparisonEnergy + report.loopEnergy + report.printEnergy;

    report.estimatedJoules = static_cast<double>(report.totalEnergyUnits) * kJoulesPerEnergyUnit;

    // Opaque calls: Time Complexity reflects only main's own loop nesting,
    // the same as before functions existed. It does not attempt to trace
    // complexity through function calls, consistent with there being no
    // other interprocedural analysis anywhere in this compiler.
    report.timeComplexity = timeComplexityFor(mainCost.maxNestingDepth);

    return report;
}
