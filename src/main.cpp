#include "Report.h"
#include "VoltageTest.h"

#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

namespace
{
struct CommandLine
{
    TestConfiguration configuration;
    std::string reportPath = "benchcheck-report.md";
    bool help = false;
};

double parseNumber(const std::string& option, const std::string& text)
{
    try
    {
        std::size_t consumed = 0;
        const double value = std::stod(text, &consumed);
        if (consumed == text.size())
        {
            return value;
        }
    }
    catch (const std::invalid_argument&) {}
    catch (const std::out_of_range&) {}
    throw std::invalid_argument(option + " requires a numeric value; received '" + text + "'.");
}

CommandLine parseArguments(int argc, char* argv[])
{
    CommandLine options;
    std::set<std::string> seen;
    for (int index = 1; index < argc; ++index)
    {
        const std::string option = argv[index];
        if (option == "--help" || option == "-h")
        {
            options.help = true;
            continue;
        }
        if (option != "--scenario" && option != "--report" && option != "--voltage" &&
            option != "--current-limit" && option != "--load-current" &&
            option != "--min-voltage" && option != "--max-voltage")
        {
            throw std::invalid_argument("Unknown option: " + option);
        }
        if (!seen.insert(option).second)
        {
            throw std::invalid_argument("Duplicate option: " + option);
        }
        if (index + 1 >= argc || std::string(argv[index + 1]).rfind("--", 0) == 0)
        {
            throw std::invalid_argument("Missing value for " + option);
        }
        const std::string value = argv[++index];
        if (option == "--scenario") options.configuration.scenario = parseScenario(value);
        else if (option == "--report")
        {
            if (value.empty()) throw std::invalid_argument("--report requires a nonempty file path.");
            options.reportPath = value;
        }
        else if (option == "--voltage") options.configuration.voltage = parseNumber(option, value);
        else if (option == "--current-limit") options.configuration.currentLimit = parseNumber(option, value);
        else if (option == "--load-current") options.configuration.loadCurrent = parseNumber(option, value);
        else if (option == "--min-voltage") options.configuration.minimumVoltage = parseNumber(option, value);
        else if (option == "--max-voltage") options.configuration.maximumVoltage = parseNumber(option, value);
    }
    return options;
}

void printHelp()
{
    std::cout
        << "Usage: benchcheck [options]\n\n"
        << "Run a simulated DC output voltage test. No hardware is accessed.\n\n"
        << "  --scenario NAME     normal (default), undervoltage, overvoltage, timeout\n"
        << "  --report PATH       Markdown report (default: benchcheck-report.md)\n"
        << "  --voltage V         Supply voltage, 0..30 V (default: 5)\n"
        << "  --current-limit A   Supply current limit, 0..5 A (default: 0.5)\n"
        << "  --load-current A    Load current, 0..current-limit A (default: 0.1)\n"
        << "  --min-voltage V     Inclusive lower limit, 0..30 V (default: 4.9)\n"
        << "  --max-voltage V     Inclusive upper limit, min-voltage..30 V (default: 5.1)\n"
        << "  --help, -h          Show this help without running a test\n\n"
        << "All settings must be finite. Existing reports are replaced; parent directories\n"
        << "must already exist. Exit codes: 0 = PASS/help, 1 = FAIL, 2 = ERROR.\n";
}
} // namespace

int main(int argc, char* argv[])
{
    try
    {
        const CommandLine options = parseArguments(argc, argv);
        if (options.help)
        {
            printHelp();
            return 0;
        }
        std::cout << u8"BenchCheck \u2014 simulated electronics test bench\n";
        SimulatedBench bench;
        const TestResult result = runVoltageTest(bench, options.configuration);
        std::cout << "Test result: " << statusName(result.status)
                  << " (" << scenarioName(options.configuration.scenario) << ")\n"
                  << "Reason: " << result.reason << '\n';
        if (result.measuredVoltage)
        {
            std::cout << "Measured voltage: " << formatNumber(*result.measuredVoltage)
                      << " V; inclusive limits: " << formatNumber(options.configuration.minimumVoltage)
                      << " to " << formatNumber(options.configuration.maximumVoltage) << " V\n";
        }
        else
        {
            std::cout << "Measured voltage: unavailable (no reading acquired).\n";
        }
        std::cout << "Final state: load " << (result.loadEnabled ? "enabled" : "disabled")
                  << ", supply " << (result.supplyEnabled ? "enabled" : "disabled") << '\n';
        // The runner has already shut down the bench, even if writing fails.
        writeReport(options.reportPath, options.configuration, result);
        std::cout << "Report: " << options.reportPath << '\n';
        return exitCode(result.status);
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return exitCode(TestStatus::Error);
    }
}
