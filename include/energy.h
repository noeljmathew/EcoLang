#ifndef ENERGY_H
#define ENERGY_H

#include <string>

#include "ir.h"

// --- Energy cost model ------------------------------------------------
// These are real, published, measured per-instruction-class energy costs
// for Arm's Cortex-M0 processor, not project-defined arbitrary weights.
// Source: Georgiou, K., Chamski, Z., Nikov, K., & Eder, K. (2021). A
// Comprehensive and Accurate Energy Model for Arm's Cortex-M0 Processor.
// arXiv:2104.01055. The model was trained and validated against 230
// benchmarks measured on physical STM32F051 hardware; every reported
// configuration in the paper achieves under 5% mean absolute percentage
// error (MAPE) against those hardware measurements. The six coefficients
// below are from Table 2, configuration [48MHz, PreFetch ON, WaitState 1]
// (the paper's highest-performance config, MAPE 4.33%):
//
//   E (nJ) = C1*kCoeffC1_Instruction + C2*kCoeffC2_Multiply
//          + C3*kCoeffC3_TakenBranch + C4*kCoeffC4_RamRead
//          + C5*kCoeffC5_RamWrite    + C6*kCoeffC6_FlashRead
//
// where C1=executed instructions (excl. multiplies), C2=multiplication
// instructions, C3=taken branches, C4=RAM reads, C5=RAM writes,
// C6=Flash reads.
constexpr double kCoeffC1_Instruction = 0.816331;  // nJ, executed instruction (excl. Muls)
constexpr double kCoeffC2_Multiply    = 2.014612;  // nJ, multiplication instruction
constexpr double kCoeffC3_TakenBranch = 1.372157;  // nJ, taken branch
constexpr double kCoeffC4_RamRead     = 1.402116;  // nJ, RAM data read
constexpr double kCoeffC5_RamWrite    = 0.835035;  // nJ, RAM write
constexpr double kCoeffC6_FlashRead   = 1.250446;  // nJ, Flash data read
// All: Georgiou et al. 2021 (arXiv:2104.01055), Table 2, config [48MHz, PreFetch ON, WaitState 1], MAPE 4.33%

// Every IR opcode's per-instruction energy cost is a fixed combination of
// the coefficients above (see energy.cpp for the exact counter mix per
// opcode, e.g. STORE = 1*C1 + 1*C5). Two entries in that mapping are NOT
// measured quantities from the paper - they are this project's own
// estimated instruction counts for constructs the paper's benchmark suite
// does not cover:
//
//   - Division: the Cortex-M0 has no hardware divide instruction; '/'
//     lowers to a software routine (__aeabi_idiv), estimated at ~40
//     instructions (40*C1). This is an estimated instruction count, not
//     a measurement.
//   - printf: not part of the paper's model at all (it is a large,
//     variable-length library call). Estimated at ~200 instructions
//     (200*C1) as a rough approximation, not a measurement.
//
// Every other opcode's cost below is a direct, measured application of
// the coefficients above.

// One deliberate model addition beyond the paper's raw per-opcode mix:
// IF_START and LOOP_START are now each charged 1*C3 (one taken branch),
// since evaluating a condition and branching on it is real, measured
// hardware cost that a comparison or loop-iteration count alone does not
// capture. Previously these opcodes cost nothing structurally. This is a
// genuine improvement in model accuracy, not an oversight - but it does
// mean every program's total energy is now somewhat higher than under
// the old flat-weight model, independent of the coefficient values
// themselves.

// No real recursion-depth analysis: when a function's cost calculation
// finds it is about to call back into a function still being computed
// (direct or mutual recursion), that function's own non-recursive tally
// is multiplied by this instead. Illustrative placeholder, not a real
// bound analysis - styled like kAssumedLoopIterations in ir.h. Orthogonal
// to the cost-per-execution model above (this is about how many times
// something executes, not what each execution costs), so it is unchanged
// by the move to real hardware coefficients.
constexpr long long kAssumedRecursionDepth = 10;

struct EnergyBreakdown {
    long long assignmentCount = 0;
    long long arithmeticCount = 0;
    long long comparisonCount = 0;  // one per COMPARE instruction executed
    long long loopIterationCount = 0;
    long long printCount = 0;
};

struct EnergyReport {
    long long totalOperations = 0;
    double totalEnergyUnits = 0.0;  // nanojoules (nJ) - see kCoeff* above
    double estimatedJoules = 0.0;
    // A simplified estimate based on maximum loop nesting depth only
    // (0 -> O(1), 1 -> O(n), 2 -> O(n^2), ...). This is not a complete
    // asymptotic analysis of arbitrary programs.
    std::string timeComplexity;
    EnergyBreakdown breakdown;

    // Energy breakdown in nanojoules (nJ). Costs now vary *within* a
    // category (e.g. '+' and '*' both count as Arithmetic but cost
    // different amounts), so each field below is accumulated directly
    // per-instruction during the scan rather than derived as
    // count * flat_constant. totalEnergyUnits is still simply the sum of
    // these five - there is no separate/second way total energy is
    // computed. See energy.cpp for the per-opcode cost mix, including
    // where the IF_START/LOOP_START branch addition and the CALL
    // instruction's own (branch + spill) cost are attributed among these
    // five categories.
    double assignmentEnergy = 0.0;
    double arithmeticEnergy = 0.0;
    double comparisonEnergy = 0.0;
    double loopEnergy = 0.0;
    double printEnergy = 0.0;
};

// Estimates the energy cost of the given IR with a single linear scan.
// Accepts any IRProgram - at integration time this will be Person C's
// optimized IR, but this function has no dependency on optimizer.cpp
// existing.
EnergyReport analyzeEnergy(const IRProgram& program);

#endif
