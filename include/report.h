#ifndef REPORT_H
#define REPORT_H

#include <string>

#include "energy.h"

// Formats an already-computed EnergyReport into a human-readable text
// report suitable for a project demonstration. This module performs no
// energy calculation of its own - analyzeEnergy() in energy.cpp is the
// only place energy values are computed. ReportGenerator only presents
// the values it is given, unchanged.
class ReportGenerator {
public:
    std::string generate(const EnergyReport& report);
};

#endif
