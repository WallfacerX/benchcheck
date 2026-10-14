// A standalone verification program, not the automated test suite (issue #8).
#include "SimulatedPowerSupply.h"

#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
void check(bool condition, const char* message)
{
    // Unlike assert(), these checks also run in Release builds.
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void checkState(const SimulatedPowerSupply& supply, double voltage,
                double currentLimit, bool enabled)
{
    check(supply.getConfiguredVoltage() == voltage, "Unexpected configured voltage");
    check(supply.getCurrentLimit() == currentLimit, "Unexpected current limit");
    check(supply.isOutputEnabled() == enabled, "Unexpected output state");
    check(supply.getOutputVoltage() == (enabled ? voltage : 0.0), "Unexpected output voltage");
}

void verifyInvalidSettings(bool enabled)
{
    SimulatedPowerSupply supply;
    supply.setVoltage(12.0);
    supply.setCurrentLimit(1.5);
    if (enabled)
    {
        supply.enableOutput();
    }

    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double invalidVoltages[] = {-1.0, 30.1, infinity, -infinity, nan};
    for (double voltage : invalidVoltages)
    {
        bool rejected = false;
        try
        {
            supply.setVoltage(voltage);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        check(rejected, "Invalid voltage was accepted");
        checkState(supply, 12.0, 1.5, enabled);
    }

    const double invalidCurrentLimits[] = {-1.0, 5.1, infinity, -infinity, nan};
    for (double currentLimit : invalidCurrentLimits)
    {
        bool rejected = false;
        try
        {
            supply.setCurrentLimit(currentLimit);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        check(rejected, "Invalid current limit was accepted");
        checkState(supply, 12.0, 1.5, enabled);
    }

    // A rejected setting must not prevent a later valid update.
    supply.setVoltage(3.3);
    supply.setCurrentLimit(0.5);
    checkState(supply, 3.3, 0.5, enabled);
}
} // namespace

int main()
{
    try
    {
        SimulatedPowerSupply supply;
        checkState(supply, 0.0, 0.0, false);

        supply.setVoltage(12.0);
        checkState(supply, 12.0, 0.0, false);
        supply.setCurrentLimit(1.5);
        checkState(supply, 12.0, 1.5, false);

        supply.enableOutput();
        checkState(supply, 12.0, 1.5, true);

        supply.disableOutput();
        checkState(supply, 12.0, 1.5, false);
        supply.disableOutput();
        checkState(supply, 12.0, 1.5, false);
        supply.enableOutput();
        checkState(supply, 12.0, 1.5, true);

        // Both endpoints are valid. Settings can also change while enabled.
        supply.setVoltage(0.0);
        supply.setCurrentLimit(0.0);
        checkState(supply, 0.0, 0.0, true);
        supply.setVoltage(30.0);
        supply.setCurrentLimit(5.0);
        checkState(supply, 30.0, 5.0, true);

        verifyInvalidSettings(false);
        verifyInvalidSettings(true);

        std::cout << "All simulated power supply checks passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Verification failed: " << error.what() << '\n';
        return 1;
    }
}
