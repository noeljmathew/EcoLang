#include "energy.h"

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

}  // namespace

EnergyReport analyzeEnergy(const IRProgram& program) {
    EnergyReport report;
    EnergyBreakdown& breakdown = report.breakdown;

    // multiplier = how many times the instruction currently being scanned
    // will actually execute, given every loop it is nested inside.
    // multiplierStack remembers the multiplier from *before* each
    // LOOP_START, so LOOP_END can restore it.
    std::vector<long long> multiplierStack;
    long long multiplier = 1;
    int nestingDepth = 0;
    int maxNestingDepth = 0;

    for (const IRInstruction& instruction : program) {
        switch (instruction.opcode) {
            case IROpcode::STORE:
                breakdown.assignmentCount += multiplier;
                break;

            case IROpcode::BINARY_OP:
                breakdown.arithmeticCount += multiplier;
                break;

            case IROpcode::PRINT:
                breakdown.printCount += multiplier;
                break;

            case IROpcode::LOOP_START:
                multiplierStack.push_back(multiplier);
                multiplier *= instruction.operandA.intValue;
                // The number of times this loop's body executes in total
                // (across all enclosing iterations) is charged as that
                // many loop-iteration units - matches the worked example
                // where "repeat 100" alone contributes 100 units.
                breakdown.loopIterationCount += multiplier;

                nestingDepth++;
                if (nestingDepth > maxNestingDepth) {
                    maxNestingDepth = nestingDepth;
                }
                break;

            case IROpcode::LOOP_END:
                multiplier = multiplierStack.back();
                multiplierStack.pop_back();
                nestingDepth--;
                break;
        }
    }

    long long loopOverheadUnits = breakdown.loopIterationCount * kLoopOverheadUnitsPerIteration;

    report.totalOperations = breakdown.assignmentCount + breakdown.arithmeticCount +
                              breakdown.comparisonCount + breakdown.loopIterationCount +
                              breakdown.printCount;

    report.totalEnergyUnits = breakdown.assignmentCount * kAssignmentCost +
                               breakdown.arithmeticCount * kArithmeticCost +
                               breakdown.comparisonCount * kComparisonCost + loopOverheadUnits +
                               breakdown.printCount * kPrintCost;

    report.estimatedJoules = static_cast<double>(report.totalEnergyUnits) * kJoulesPerEnergyUnit;
    report.timeComplexity = timeComplexityFor(maxNestingDepth);

    return report;
}
