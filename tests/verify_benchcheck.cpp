// Standalone checks for issues #3-#6; the testing framework belongs to issue #8.
#include "Report.h"
#include "SimulatedDigitalMultimeter.h"
#include "VoltageTest.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{
void check(bool condition, const std::string& message)
{
    if (!condition) throw std::runtime_error(message);
}

void checkNear(double actual, double expected, const std::string& message)
{
    check(std::abs(actual - expected) < 1e-12, message);
}

template<typename Exception, typename Operation>
void expectException(Operation operation, const std::string& message)
{
    bool caught = false;
    try { operation(); }
    catch (const Exception&) { caught = true; }
    check(caught, message);
}

void checkDisabled(const SimulatedBench& bench, const TestResult& result)
{
    check(!bench.supply().isOutputEnabled(), "Supply still enabled");
    check(!bench.load().isInputEnabled(), "Load still enabled");
    check(!result.supplyEnabled && !result.loadEnabled, "Incorrect final-state snapshot");
    check(bench.getOutputVoltage() == 0.0, "Voltage remains after shutdown");
    check(bench.getLoadCurrent() == 0.0, "Current remains after shutdown");
}

void verifyLoadAndBench()
{
    SimulatedBench bench;
    SimulatedDigitalMultimeter meter(bench);
    check(!bench.load().isInputEnabled(), "Load enabled by default");
    check(bench.load().getConfiguredCurrent() == 0.0, "Nonzero default current");
    check(meter.measureVoltage() == 0.0, "Nonzero default voltage");
    bench.supply().setVoltage(5.0);
    bench.supply().setCurrentLimit(0.5);
    bench.load().setCurrent(0.1);
    check(!bench.load().isInputEnabled(), "Setting current enabled the load");
    check(bench.getLoadCurrent() == 0.0, "Disabled load draws current");
    bench.supply().enableOutput();
    bench.load().enableInput();
    check(meter.measureVoltage() == 5.0, "Expected 5 V under a 0.1 A load");
    check(bench.getLoadCurrent() == 0.1, "Expected 0.1 A load current");

    bench.supply().setVoltage(12.0);
    check(meter.measureVoltage() == 12.0, "Meter does not follow live bench voltage");
    bench.load().setCurrent(0.5);
    check(bench.getLoadCurrent() == 0.5, "Demand equal to current limit rejected");
    bench.load().setCurrent(0.6);
    expectException<std::runtime_error>([&] { meter.measureVoltage(); }, "Overcurrent voltage read accepted");
    expectException<std::runtime_error>([&] { bench.getLoadCurrent(); }, "Overcurrent current read accepted");
    bench.load().disableInput();
    bench.load().disableInput();
    check(bench.getLoadCurrent() == 0.0, "Disabling load did not stop current");
    check(meter.measureVoltage() == 12.0, "Disabling load changed supply voltage");
    check(bench.load().getConfiguredCurrent() == 0.6, "Disabling load lost its setting");
    bench.load().enableInput();
    bench.supply().disableOutput();
    check(meter.measureVoltage() == 0.0, "Disabled supply has measured voltage");
    check(bench.getLoadCurrent() == 0.0, "Disabled supply has load current");

    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    for (bool enabled : {false, true})
    {
        if (enabled) bench.load().enableInput();
        else bench.load().disableInput();
        for (double invalid : {-0.1, 5.1, infinity, -infinity, nan})
        {
            expectException<std::invalid_argument>([&] { bench.load().setCurrent(invalid); },
                                                   "Invalid load setting accepted");
            check(bench.load().getConfiguredCurrent() == 0.6, "Invalid load setting changed current");
            check(bench.load().isInputEnabled() == enabled, "Invalid load setting changed input state");
        }
    }
    bench.load().setCurrent(0.0);
    check(bench.load().getConfiguredCurrent() == 0.0, "Load lower endpoint rejected");
    bench.load().setCurrent(5.0);
    check(bench.load().getConfiguredCurrent() == 5.0, "Load upper endpoint rejected");
    expectException<std::invalid_argument>([&] { SimulatedDigitalMultimeter invalid(bench, nan); },
                                           "Non-finite meter offset accepted");
    SimulatedDigitalMultimeter timeout(bench, 0.0, true);
    expectException<InstrumentTimeout>([&] { timeout.measureVoltage(); }, "Missing typed timeout error");
}

void verifyScenarios()
{
    SimulatedBench bench;
    TestConfiguration configuration;
    for (Scenario scenario : {Scenario::Normal, Scenario::Undervoltage, Scenario::Overvoltage, Scenario::Timeout})
    {
        configuration.scenario = scenario;
        std::string previousReport;
        for (int repeat = 0; repeat < 3; ++repeat)
        {
            const auto result = runVoltageTest(bench, configuration);
            const auto expected = scenario == Scenario::Normal ? TestStatus::Pass :
                                  scenario == Scenario::Timeout ? TestStatus::Error : TestStatus::Fail;
            check(result.status == expected, "Incorrect scenario outcome");
            check(exitCode(result.status) == (expected == TestStatus::Pass ? 0 : expected == TestStatus::Fail ? 1 : 2),
                  "Incorrect exit code");
            checkDisabled(bench, result);
            check(bench.supply().getConfiguredVoltage() == 5.0, "Runner failed to configure voltage");
            check(bench.supply().getCurrentLimit() == 0.5, "Runner failed to configure current limit");
            check(bench.load().getConfiguredCurrent() == 0.1, "Runner failed to configure load");
            const std::string report = formatReport(configuration, result);
            check(report.find("Scenario: **" + std::string(scenarioName(scenario)) + "**") != std::string::npos,
                  "Report is missing scenario");
            check(report.find("Result: **" + std::string(statusName(expected)) + "**") != std::string::npos,
                  "Report is missing result");
            check(report.find(result.reason) != std::string::npos && !result.reason.empty(), "Missing reason");
            check(report.find("DC output voltage under load") != std::string::npos, "Missing test name");
            check(report.find("All instruments are simulated") != std::string::npos, "Missing simulation notice");
            check(report.find("0.5 A") != std::string::npos && report.find("0.1 A") != std::string::npos,
                  "Missing current settings or units");
            check(report.find("4.9 V") != std::string::npos && report.find("5.1 V") != std::string::npos,
                  "Missing acceptance limits or units");
            check(report.find("Load input: disabled") != std::string::npos &&
                  report.find("Supply output: disabled") != std::string::npos, "Missing shutdown state");
            if (scenario == Scenario::Timeout)
            {
                check(!result.measuredVoltage, "Timeout fabricated a reading");
                check(report.find("Measured voltage: unavailable") != std::string::npos, "Missing unavailable reading");
                check(report.find("Measured voltage: 0") == std::string::npos, "Missing reading shown as zero");
            }
            else
            {
                check(result.measuredVoltage.has_value(), "Completed test lost its reading");
                const double expectedVoltage = scenario == Scenario::Normal ? 5.0 :
                                               scenario == Scenario::Undervoltage ? 4.7 : 5.3;
                checkNear(*result.measuredVoltage, expectedVoltage, "Incorrect deterministic reading");
            }
            if (repeat > 0) check(report == previousReport, "Scenario report is not repeatable");
            previousReport = report;
        }
    }

    // Faults must still exceed the limits when the acceptance band is widened.
    configuration.minimumVoltage = 0.0;
    configuration.maximumVoltage = 30.0;
    for (Scenario scenario : {Scenario::Undervoltage, Scenario::Overvoltage})
    {
        configuration.scenario = scenario;
        const auto result = runVoltageTest(bench, configuration);
        check(result.status == TestStatus::Fail, "Voltage fault passed with custom limits");
        checkDisabled(bench, result);
    }
    check(parseScenario("normal") == Scenario::Normal, "Cannot parse normal scenario");
    check(parseScenario("undervoltage") == Scenario::Undervoltage, "Cannot parse undervoltage scenario");
    check(parseScenario("overvoltage") == Scenario::Overvoltage, "Cannot parse overvoltage scenario");
    check(parseScenario("timeout") == Scenario::Timeout, "Cannot parse timeout scenario");
    expectException<std::invalid_argument>([] { parseScenario("unknown"); }, "Unknown scenario accepted");
}

void verifyBoundaries()
{
    SimulatedBench bench;
    TestConfiguration configuration;
    for (double voltage : {4.9, 5.1})
    {
        configuration.voltage = voltage;
        const auto result = runVoltageTest(bench, configuration);
        check(result.status == TestStatus::Pass, "Inclusive voltage boundary failed");
        checkDisabled(bench, result);
    }
    for (double voltage : {std::nextafter(4.9, 0.0), std::nextafter(5.1, 30.0)})
    {
        configuration.voltage = voltage;
        const auto result = runVoltageTest(bench, configuration);
        check(result.status == TestStatus::Fail, "Voltage just outside boundary passed");
        check(std::stod(formatNumber(*result.measuredVoltage)) == voltage, "Report rounded a failing reading");
        const std::string report = formatReport(configuration, result);
        check(report.find("Measured voltage: " + formatNumber(voltage) + " V") != std::string::npos,
              "Report lost measurement precision");
        checkDisabled(bench, result);
    }
    for (double voltage : {0.0, 30.0})
    {
        configuration.voltage = voltage;
        configuration.minimumVoltage = voltage;
        configuration.maximumVoltage = voltage;
        configuration.currentLimit = voltage == 0.0 ? 0.0 : 5.0;
        configuration.loadCurrent = configuration.currentLimit;
        const auto result = runVoltageTest(bench, configuration);
        check(result.status == TestStatus::Pass, "Valid instrument endpoint rejected");
        check(result.measuredVoltage && *result.measuredVoltage == voltage, "Incorrect endpoint reading");
        checkDisabled(bench, result);
    }
}

void verifyRejection(const TestConfiguration& configuration, const std::string& field)
{
    SimulatedBench bench;
    bench.supply().setVoltage(12.0);
    bench.supply().setCurrentLimit(1.5);
    bench.load().setCurrent(1.0);
    const auto result = runVoltageTest(bench, configuration);
    check(result.status == TestStatus::Error, "Invalid configuration did not produce ERROR");
    check(result.reason.find(field) != std::string::npos, "Validation error did not name " + field);
    check(!result.measuredVoltage, "Rejected configuration produced a reading");
    checkDisabled(bench, result);
    check(bench.supply().getConfiguredVoltage() == 12.0, "Validation partially configured voltage");
    check(bench.supply().getCurrentLimit() == 1.5, "Validation partially configured current limit");
    check(bench.load().getConfiguredCurrent() == 1.0, "Validation partially configured load");
}

void verifyValidation()
{
    struct Field
    {
        double TestConfiguration::*member;
        const char* name;
        double maximum;
    };
    const Field fields[] = {
        {&TestConfiguration::voltage, "voltage", 30.0},
        {&TestConfiguration::currentLimit, "current-limit", 5.0},
        {&TestConfiguration::loadCurrent, "load-current", 5.0},
        {&TestConfiguration::minimumVoltage, "min-voltage", 30.0},
        {&TestConfiguration::maximumVoltage, "max-voltage", 30.0}
    };
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    for (const auto& field : fields)
    {
        for (double invalid : {-0.1, field.maximum + 0.1, infinity, -infinity, nan})
        {
            TestConfiguration configuration;
            configuration.*(field.member) = invalid;
            verifyRejection(configuration, field.name);
        }
    }
    TestConfiguration configuration;
    configuration.minimumVoltage = 6.0;
    verifyRejection(configuration, "min-voltage");
    configuration = TestConfiguration{};
    configuration.loadCurrent = 0.6;
    verifyRejection(configuration, "load-current");
    configuration = TestConfiguration{};
    configuration.scenario = static_cast<Scenario>(99);
    verifyRejection(configuration, "scenario");

    // A reused bench must also be disabled when the next configuration is invalid.
    SimulatedBench bench;
    bench.supply().enableOutput();
    bench.load().enableInput();
    checkDisabled(bench, runVoltageTest(bench, configuration));
    const auto recovered = runVoltageTest(bench, TestConfiguration{});
    check(recovered.status == TestStatus::Pass, "Bench cannot recover after validation failure");
    checkDisabled(bench, recovered);
}

void verifyReportFailure()
{
    SimulatedBench bench;
    const TestConfiguration configuration;
    const auto result = runVoltageTest(bench, configuration);
    // A directory cannot be opened as a report file on supported platforms.
    expectException<std::runtime_error>([&] { writeReport(".", configuration, result); },
                                        "Report open failure was ignored");
    checkDisabled(bench, result);
}
} // namespace

int main()
{
    try
    {
        verifyLoadAndBench();
        verifyScenarios();
        verifyBoundaries();
        verifyValidation();
        verifyReportFailure();
        std::cout << "All bench, voltage test, validation, scenario, and report checks passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Verification failed: " << error.what() << '\n';
        return 1;
    }
}
