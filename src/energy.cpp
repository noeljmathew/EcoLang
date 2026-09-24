#include "energy.h"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

// --- Precomputed per-IR-opcode costs (nJ) ------------------------------
// Each constant below is a fixed combination of the kCoeff* hardware
// coefficients in energy.h, precomputed exactly as specified rather than
// recomputed at runtime, so the literal values here are the authoritative
// numbers (not the coefficient arithmetic, which is shown only in
// comments for traceability). All measured except kCostDivision and
// kCostPrint, which are explicitly documented estimates (see energy.h).

// STORE = 1*C1 + 1*C5 (a store is one executed instruction plus one RAM write).
constexpr double kCostStore = 1.651366;

// BINARY_OP '+'/'-' = 1*C1 + 2*C4 (one instruction, two operand RAM reads).
constexpr double kCostBinaryAddSub = 3.620563;

// BINARY_OP '*' = 1*C2 + 2*C4 (a multiply instruction, two operand RAM reads).
constexpr double kCostBinaryMul = 4.818844;

// BINARY_OP '/' = ESTIMATED, not measured. The Cortex-M0 has no hardware
// divide instruction; '/' lowers to a software routine (__aeabi_idiv),
// estimated at ~40 instructions: 40*C1. This is an estimated instruction
// count, not a hardware measurement.
constexpr double kCostDivision = 32.653240;

// COMPARE = 1*C1 + 2*C4 (one instruction, two operand RAM reads).
constexpr double kCostCompare = 3.620563;

// IF_START / LOOP_START (per taken branch) = 1*C3. See energy.h for why
// this is a deliberate model addition (previously these cost nothing).
constexpr double kCostTakenBranch = 1.372157;

// CALL's own instruction cost (distinct from the callee body's cost,
// which is added separately via the existing call-graph expansion) =
// 1*C3 (the call is itself a taken branch) + 4*C5 (assumed
// register/stack spill around the call).
constexpr double kCostCall = 4.712297;

// PRINT = ESTIMATED, not measured. printf is a large, variable-length
// library call that is not part of the paper's model at all; modeled as
// an estimated ~200 instructions: 200*C1. Rough approximation, not a
// hardware measurement.
constexpr double kCostPrint = 163.266200;

double costForBinaryOp(char operatorSymbol, int line) {
    switch (operatorSymbol) {
        case '+':
        case '-':
            return kCostBinaryAddSub;
        case '*':
            return kCostBinaryMul;
        case '/':
            return kCostDivision;
        default:
            throw std::runtime_error("Energy analysis error on line " + std::to_string(line) +
                                     ": unsupported operator '" + operatorSymbol + "'.");
    }
}

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

// Nanojoule accumulators, one per EnergyBreakdown category, kept in
// parallel with (not inside) EnergyBreakdown itself - EnergyBreakdown's
// fields stay pure operation counts, exactly as before. IF_START's
// branch cost is attributed to comparisonEnergy (every IF_START is
// immediately preceded by exactly one COMPARE producing its condition -
// see ir.cpp generateIf - so the two always co-occur 1:1 at the same
// loop-nesting multiplier). CALL's own (branch + spill) cost is
// attributed to assignmentEnergy (4 of its 6 cost units are RAM writes,
// the same kind of cost STORE already represents there). Neither
// IF_START nor CALL touch any EnergyBreakdown *count* field - those
// remain computed exactly as before.
struct EnergyByCategory {
    double assignmentEnergy = 0.0;
    double arithmeticEnergy = 0.0;
    double comparisonEnergy = 0.0;
    double loopEnergy = 0.0;
    double printEnergy = 0.0;
};

void addScaled(EnergyBreakdown& total, const EnergyBreakdown& part, long long multiplier) {
    total.assignmentCount += part.assignmentCount * multiplier;
    total.arithmeticCount += part.arithmeticCount * multiplier;
    total.comparisonCount += part.comparisonCount * multiplier;
    total.loopIterationCount += part.loopIterationCount * multiplier;
    total.printCount += part.printCount * multiplier;
}

void addScaledEnergy(EnergyByCategory& total, const EnergyByCategory& part, long long multiplier) {
    double m = static_cast<double>(multiplier);
    total.assignmentEnergy += part.assignmentEnergy * m;
    total.arithmeticEnergy += part.arithmeticEnergy * m;
    total.comparisonEnergy += part.comparisonEnergy * m;
    total.loopEnergy += part.loopEnergy * m;
    total.printEnergy += part.printEnergy * m;
}

void multiplyInPlace(EnergyBreakdown& breakdown, long long factor) {
    breakdown.assignmentCount *= factor;
    breakdown.arithmeticCount *= factor;
    breakdown.comparisonCount *= factor;
    breakdown.loopIterationCount *= factor;
    breakdown.printCount *= factor;
}

void multiplyInPlace(EnergyByCategory& energy, long long factor) {
    double f = static_cast<double>(factor);
    energy.assignmentEnergy *= f;
    energy.arithmeticEnergy *= f;
    energy.comparisonEnergy *= f;
    energy.loopEnergy *= f;
    energy.printEnergy *= f;
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
    EnergyByCategory energy;
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
// by the caller afterward. Energy is accumulated directly per-instruction
// (count * flat_constant no longer holds once costs vary within a
// category), in parallel with the pre-existing operation counts.
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
                result.energy.assignmentEnergy += static_cast<double>(multiplier) * kCostStore;
                break;

            case IROpcode::BINARY_OP:
                result.breakdown.arithmeticCount += multiplier;
                result.energy.arithmeticEnergy +=
                    static_cast<double>(multiplier) *
                    costForBinaryOp(instruction.operatorSymbol, instruction.line);
                break;

            case IROpcode::PRINT:
                result.breakdown.printCount += multiplier;
                result.energy.printEnergy += static_cast<double>(multiplier) * kCostPrint;
                break;

            case IROpcode::COMPARE:
                result.breakdown.comparisonCount += multiplier;
                result.energy.comparisonEnergy += static_cast<double>(multiplier) * kCostCompare;
                break;

            case IROpcode::IF_START:
                // Deliberate model addition (see energy.h): a taken
                // branch, attributed to Comparisons - see the
                // EnergyByCategory comment above for why. Does not touch
                // any EnergyBreakdown count field.
                result.energy.comparisonEnergy +=
                    static_cast<double>(multiplier) * kCostTakenBranch;
                break;

            case IROpcode::RETURN:
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
                // Deliberate model addition (see energy.h): each taken
                // loop-branch costs 1*C3, same coefficient as IF_START
                // above. loopIterationCount already equals the number of
                // iterations at this nesting level, so this is that same
                // count times the branch coefficient.
                result.energy.loopEnergy += static_cast<double>(multiplier) * kCostTakenBranch;

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
                // The CALL instruction's own cost (branch + assumed
                // spill), independent of whatever the callee's body
                // itself costs below. Attributed to Assignments - see
                // the EnergyByCategory comment above for why. Does not
                // touch any EnergyBreakdown count field.
                result.energy.assignmentEnergy += static_cast<double>(multiplier) * kCostCall;

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
                addScaledEnergy(result.energy, calleeCost.energy, multiplier);
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
        multiplyInPlace(cost.energy, kAssumedRecursionDepth);
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

    // Energy breakdown, in nanojoules: each category's energy is exactly
    // what was accumulated directly per-instruction during the scan
    // above (see EnergyByCategory) - there is no separate count * cost
    // step here, since costs vary within a category. totalEnergyUnits is
    // simply their sum, so there is one single source of truth for it.
    report.assignmentEnergy = mainCost.energy.assignmentEnergy;
    report.arithmeticEnergy = mainCost.energy.arithmeticEnergy;
    report.comparisonEnergy = mainCost.energy.comparisonEnergy;
    report.loopEnergy = mainCost.energy.loopEnergy;
    report.printEnergy = mainCost.energy.printEnergy;

    report.totalEnergyUnits = report.assignmentEnergy + report.arithmeticEnergy +
                               report.comparisonEnergy + report.loopEnergy + report.printEnergy;

    // Real unit conversion, not an assumption: totalEnergyUnits is
    // genuinely in nanojoules (nJ) now (see energy.h), so this is just
    // nJ -> J, modulo the two documented estimated entries (division,
    // printf) within an otherwise-measured model.
    report.estimatedJoules = report.totalEnergyUnits * 1e-9;

    // Opaque calls: Time Complexity reflects only main's own loop nesting,
    // the same as before functions existed. It does not attempt to trace
    // complexity through function calls, consistent with there being no
    // other interprocedural analysis anywhere in this compiler.
    report.timeComplexity = timeComplexityFor(mainCost.maxNestingDepth);

    return report;
}
