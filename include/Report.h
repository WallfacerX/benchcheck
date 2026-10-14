#ifndef BENCHCHECK_REPORT_H
#define BENCHCHECK_REPORT_H

#include "VoltageTest.h"

#include <filesystem>
#include <string>

// Shortest round-trip representation, so a reading just outside a limit does
// not round to that limit in the console or report.
std::string formatNumber(double value);
std::string formatReport(const TestConfiguration& configuration, const TestResult& result);
// Throws std::runtime_error on open, write, flush, or close failure.
void writeReport(const std::filesystem::path& path,
                 const TestConfiguration& configuration, const TestResult& result);

#endif
