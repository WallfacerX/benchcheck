#include "VoltageTest.h"

#include "SimulatedDigitalMultimeter.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
void shutdown(SimulatedBench& bench) noexcept
{
    bench.load().disableInput();
    bench.supply().disableOutput();
}

class OutputShutdown
{
public:
    explicit OutputShutdown(SimulatedBench& bench) : bench_(bench) {}
    ~OutputShutdown() { shutdown(bench_); }
    OutputShutdown(const OutputShutdown&) = delete;
    OutputShutdown& operator=(const OutputShutdown&) = delete;

private:
    SimulatedBench& bench_;
};

void validateRange(double value, double minimum, double maximum, const char* message)
{
    if (!std::isfinite(value) || value < minimum || value > maximum)
    {
        throw std::invalid_argument(message);
    }
}
} // namespace

const char* scenarioName(Scenario scenario)
{
    switch (scenario)
    {
    case Scenario::Normal: return "normal";
    case Scenario::Undervoltage: return "undervoltage";
    case Scenario::Overvoltage: return "overvoltage";
    case Scenario::Timeout: return "timeout";
    }
    return "unknown";
}

Scenario parseScenario(const std::string& name)
{
    if (name == "normal") return Scenario::Normal;
    if (name == "undervoltage") return Scenario::Undervoltage;
    if (name == "overvoltage") return Scenario::Overvoltage;
    if (name == "timeout") return Scenario::Timeout;
    throw std::invalid_argument("scenario must be normal, undervoltage, overvoltage, or timeout.");
}

const char* statusName(TestStatus status)
{
    switch (status)
    {
    case TestStatus::Pass: return "PASS";
    case TestStatus::Fail: return "FAIL";
    case TestStatus::Error: return "ERROR";
    }
    return "ERROR";
}

int exitCode(TestStatus status)
{
    switch (status)
    {
    case TestStatus::Pass: return 0;
    case TestStatus::Fail: return 1;
    case TestStatus::Error: return 2;
    }
    return 2;
}

void validateConfiguration(const TestConfiguration& configuration)
{
    validateRange(configuration.voltage, SimulatedPowerSupply::kMinVoltage,
                  SimulatedPowerSupply::kMaxVoltage,
                  "voltage must be finite and between 0 and 30 V.");
    validateRange(configuration.currentLimit, SimulatedPowerSupply::kMinCurrentLimit,
                  SimulatedPowerSupply::kMaxCurrentLimit,
                  "current-limit must be finite and between 0 and 5 A.");
    validateRange(configuration.loadCurrent, SimulatedLoad::kMinCurrent,
                  SimulatedLoad::kMaxCurrent,
                  "load-current must be finite and between 0 and 5 A.");
    validateRange(configuration.minimumVoltage, SimulatedPowerSupply::kMinVoltage,
                  SimulatedPowerSupply::kMaxVoltage,
                  "min-voltage must be finite and between 0 and 30 V.");
    validateRange(configuration.maximumVoltage, SimulatedPowerSupply::kMinVoltage,
                  SimulatedPowerSupply::kMaxVoltage,
                  "max-voltage must be finite and between 0 and 30 V.");
    if (configuration.minimumVoltage > configuration.maximumVoltage)
    {
        throw std::invalid_argument("min-voltage must be less than or equal to max-voltage.");
    }
    if (configuration.loadCurrent > configuration.currentLimit)
    {
        throw std::invalid_argument("load-current must not exceed current-limit.");
    }
    switch (configuration.scenario)
    {
    case Scenario::Normal:
    case Scenario::Undervoltage:
    case Scenario::Overvoltage:
    case Scenario::Timeout:
        break;
    default:
        throw std::invalid_argument("scenario must be normal, undervoltage, overvoltage, or timeout.");
    }
}

TestResult runVoltageTest(SimulatedBench& bench, const TestConfiguration& configuration)
{
    TestResult result;
    shutdown(bench);
    try
    {
        OutputShutdown cleanup(bench);
        validateConfiguration(configuration);
        bench.supply().setVoltage(configuration.voltage);
        bench.supply().setCurrentLimit(configuration.currentLimit);
        bench.load().setCurrent(configuration.loadCurrent);

        // Move the reading at least 0.2 V beyond the selected acceptance band,
        // including when the caller changes the nominal voltage or limits.
        double offset = 0.0;
        if (configuration.scenario == Scenario::Undervoltage)
        {
            offset = std::min(0.0, configuration.minimumVoltage - configuration.voltage) - 0.2;
        }
        else if (configuration.scenario == Scenario::Overvoltage)
        {
            offset = std::max(0.0, configuration.maximumVoltage - configuration.voltage) + 0.2;
        }
        SimulatedDigitalMultimeter meter(bench, offset, configuration.scenario == Scenario::Timeout);

        bench.supply().enableOutput();
        bench.load().enableInput();
        result.measuredVoltage = meter.measureVoltage();
        if (*result.measuredVoltage < configuration.minimumVoltage)
        {
            result.status = TestStatus::Fail;
            result.reason = "Measured voltage is below min-voltage.";
        }
        else if (*result.measuredVoltage > configuration.maximumVoltage)
        {
            result.status = TestStatus::Fail;
            result.reason = "Measured voltage is above max-voltage.";
        }
        else
        {
            result.status = TestStatus::Pass;
            result.reason = "Measured voltage is within the inclusive acceptance limits.";
        }
    }
    catch (const std::exception& error)
    {
        result.status = TestStatus::Error;
        result.reason = error.what();
    }
    result.supplyEnabled = bench.supply().isOutputEnabled();
    result.loadEnabled = bench.load().isInputEnabled();
    return result;
}
