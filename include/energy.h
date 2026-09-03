#ifndef ENERGY_H
#define ENERGY_H

#include <string>

#include "ir.h"

// --- Energy cost model ------------------------------------------------
// These are estimated computational costs (matching the project's own
// cost table), not real hardware energy measurements.

constexpr long long kAssignmentCost = 1;
constexpr long long kArithmeticCost = 1;
constexpr long long kComparisonCost = 1;
constexpr long long kPrintCost = 5;

// The requirements table lists "Loop Overhead = 2-3", but the worked
// example directly beneath it (repeat 100 -> "Loop overhead = 100") only
// makes sense if loop overhead is 1 unit per iteration, not a flat 2-3
// per loop. This implementation follows the worked example and treats
// that as the documented project assumption. Kept as a single named
// constant so it can be changed to 2 or 3 later without touching any
// analysis logic.
constexpr long long kLoopOverheadUnitsPerIteration = 1;

// Illustrative unit-to-Joules conversion only. The requirements do not
// specify a real conversion factor, and this is NOT a hardware-calibrated
// measurement - it exists purely to give the final report a
// human-readable "estimated Joules" figure.
constexpr double kJoulesPerEnergyUnit = 1e-7;

struct EnergyBreakdown {
    long long assignmentCount = 0;
    long long arithmeticCount = 0;
    long long comparisonCount = 0;  // always 0 today - the IR has no comparison opcode yet
    long long loopIterationCount = 0;
    long long printCount = 0;
};

struct EnergyReport {
    long long totalOperations = 0;
    long long totalEnergyUnits = 0;
    double estimatedJoules = 0.0;
    // A simplified estimate based on maximum loop nesting depth only
    // (0 -> O(1), 1 -> O(n), 2 -> O(n^2), ...). This is not a complete
    // asymptotic analysis of arbitrary programs.
    std::string timeComplexity;
    EnergyBreakdown breakdown;

    // Energy breakdown: each category's count (above) multiplied by its
    // cost from the cost model above. totalEnergyUnits is just the sum of
    // these five - there is no separate/second way total energy is computed.
    long long assignmentEnergy = 0;
    long long arithmeticEnergy = 0;
    long long comparisonEnergy = 0;
    long long loopEnergy = 0;
    long long printEnergy = 0;
};

// Estimates the energy cost of the given IR with a single linear scan.
// Accepts any IRProgram - at integration time this will be Person C's
// optimized IR, but this function has no dependency on optimizer.cpp
// existing.
EnergyReport analyzeEnergy(const IRProgram& program);

#endif
