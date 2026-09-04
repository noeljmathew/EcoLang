#include "energy.h"

#include <stdexcept>
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
                // One PRINT instruction is one print operation regardless of
                // how many printf arguments it carries (operandList size is
                // not part of the cost model).
                breakdown.printCount += multiplier;
                break;

            case IROpcode::RETURN:
                // A control-flow exit, not a computation - any energy spent
                // producing its value was already counted by the STORE/
                // BINARY_OP instructions that computed it. No separate cost,
                // and report.cpp has no row to show one even if there were.
                break;

            case IROpcode::COMPARE:
                breakdown.comparisonCount += multiplier;
                break;

            case IROpcode::IF_START:
            case IROpcode::ELSE:
            case IROpcode::IF_END:
                // Structural markers only. Both branches' instructions sit
                // one after another in this flat IR, so the linear scan
                // counts both unconditionally just by walking past them -
                // an intentional worst-case over-approximation, not a real
                // control-flow simulation. No multiplier change here,
                // unlike LOOP_START/LOOP_END.
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
                if (multiplierStack.empty()) {
                    throw std::runtime_error("Energy analysis error on line " +
                                             std::to_string(instruction.line) +
                                             ": unmatched LOOP_END (no matching LOOP_START).");
                }
                multiplier = multiplierStack.back();
                multiplierStack.pop_back();
                nestingDepth--;
                break;
        }
    }

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
    report.timeComplexity = timeComplexityFor(maxNestingDepth);

    return report;
}
