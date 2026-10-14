#include "Report.h"

#include <charconv>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <system_error>

std::string formatNumber(double value)
{
    char buffer[64];
    const auto conversion = std::to_chars(buffer, buffer + sizeof(buffer), value);
    if (conversion.ec != std::errc{})
    {
        throw std::runtime_error("Cannot format numeric value.");
    }
    return std::string(buffer, conversion.ptr);
}

std::string formatReport(const TestConfiguration& configuration, const TestResult& result)
{
    std::ostringstream report;
    report.imbue(std::locale::classic());
    report << "# BenchCheck report\n\n"
           << "Test: DC output voltage under load\n\n"
           << "All instruments are simulated: DC power supply, constant-current electronic load, "
              "and digital multimeter. No hardware is connected.\n\n"
           << "Scenario: **" << scenarioName(configuration.scenario) << "**\n\n"
           << "Result: **" << statusName(result.status) << "**\n\n"
           << "Reason: " << result.reason << "\n\n"
           << "## Requested settings\n\n"
           << "These are requested values; invalid settings are rejected before configuration.\n\n"
           << "| Setting | Value |\n| --- | --- |\n"
           << "| Supply voltage | " << formatNumber(configuration.voltage) << " V |\n"
           << "| Supply current limit | " << formatNumber(configuration.currentLimit) << " A |\n"
           << "| Load current | " << formatNumber(configuration.loadCurrent) << " A |\n"
           << "| Minimum accepted voltage (inclusive) | " << formatNumber(configuration.minimumVoltage) << " V |\n"
           << "| Maximum accepted voltage (inclusive) | " << formatNumber(configuration.maximumVoltage) << " V |\n\n"
           << "## Measurement\n\n";
    if (result.measuredVoltage)
    {
        report << "Measured voltage: " << formatNumber(*result.measuredVoltage) << " V\n\n";
    }
    else
    {
        report << "Measured voltage: unavailable (no reading acquired).\n\n";
    }
    report << "## Final instrument state\n\n"
           << "- Load input: " << (result.loadEnabled ? "enabled" : "disabled") << "\n"
           << "- Supply output: " << (result.supplyEnabled ? "enabled" : "disabled") << "\n";
    return report.str();
}

void writeReport(const std::filesystem::path& path,
                 const TestConfiguration& configuration, const TestResult& result)
{
    const std::string report = formatReport(configuration, result);
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file)
    {
        throw std::runtime_error("Cannot open report for writing: " + path.string());
    }
    file << report;
    file.flush();
    const bool writeFailed = !file;
    file.close();
    if (writeFailed || !file)
    {
        throw std::runtime_error("Cannot finish writing report: " + path.string());
    }
}
