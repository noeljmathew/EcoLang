#include "report.h"

#include <iomanip>
#include <sstream>

namespace {

// Writes one "label   : value" row, padding the label so the colons
// line up regardless of how long each label is.
void writeRow(std::ostringstream& out, const std::string& label, const std::string& value) {
    out << std::left << std::setw(20) << label << ": " << value << '\n';
}

}  // namespace

std::string ReportGenerator::generate(const EnergyReport& report) {
    std::ostringstream out;

    out << "========================================\n";
    out << "        EcoLang Energy Report\n";
    out << "========================================\n\n";

    out << "Operation Counts\n";
    out << "----------------\n";
    writeRow(out, "Assignments", std::to_string(report.breakdown.assignmentCount));
    writeRow(out, "Arithmetic Ops", std::to_string(report.breakdown.arithmeticCount));
    writeRow(out, "Comparisons", std::to_string(report.breakdown.comparisonCount));
    writeRow(out, "Loop Iterations", std::to_string(report.breakdown.loopIterationCount));
    writeRow(out, "Print Statements", std::to_string(report.breakdown.printCount));
    writeRow(out, "Total Operations", std::to_string(report.totalOperations));
    out << '\n';

    out << "Energy Breakdown\n";
    out << "----------------\n";
    writeRow(out, "Assignment Energy", std::to_string(report.assignmentEnergy));
    writeRow(out, "Arithmetic Energy", std::to_string(report.arithmeticEnergy));
    writeRow(out, "Comparison Energy", std::to_string(report.comparisonEnergy));
    writeRow(out, "Loop Energy", std::to_string(report.loopEnergy));
    writeRow(out, "Print Energy", std::to_string(report.printEnergy));
    out << '\n';

    writeRow(out, "Total Energy Units", std::to_string(report.totalEnergyUnits));

    // Fixed to 7 decimal places so the Joules figure - which comes from
    // multiplying by the illustrative 1e-7 kJoulesPerEnergyUnit constant
    // in energy.h - reliably prints as a plain decimal (e.g. 0.0001218)
    // instead of switching to scientific notation.
    std::ostringstream joules;
    joules << std::fixed << std::setprecision(7) << report.estimatedJoules << " J";
    writeRow(out, "Estimated Energy", joules.str());
    out << '\n';

    out << "Complexity\n";
    out << "----------\n";
    writeRow(out, "Time Complexity", report.timeComplexity);

    return out.str();
}
