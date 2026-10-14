#ifndef BENCHCHECK_VOLTAGE_TEST_H
#define BENCHCHECK_VOLTAGE_TEST_H

#include "SimulatedBench.h"

#include <optional>
#include <string>

enum class Scenario { Normal, Undervoltage, Overvoltage, Timeout };
enum class TestStatus { Pass, Fail, Error };

struct TestConfiguration
{
    double voltage = 5.0;      // V, [0, 30]
    double currentLimit = 0.5; // A, [0, 5]
    double loadCurrent = 0.1;  // A, [0, currentLimit]
    double minimumVoltage = 4.9; // V, inclusive, [0, 30]
    double maximumVoltage = 5.1; // V, inclusive, [minimumVoltage, 30]
    Scenario scenario = Scenario::Normal;
};

struct TestResult
{
    TestStatus status = TestStatus::Error;
    std::optional<double> measuredVoltage;
    std::string reason;
    bool supplyEnabled = false;
    bool loadEnabled = false;
};

const char* scenarioName(Scenario scenario);
Scenario parseScenario(const std::string& name);
const char* statusName(TestStatus status);
int exitCode(TestStatus status);

// Throws std::invalid_argument naming the first invalid field.
void validateConfiguration(const TestConfiguration& configuration);

// Starts with outputs disabled; validates all settings before configuring or
// enabling either instrument. Both outputs are disabled before returning.
TestResult runVoltageTest(SimulatedBench& bench, const TestConfiguration& configuration);

#endif
