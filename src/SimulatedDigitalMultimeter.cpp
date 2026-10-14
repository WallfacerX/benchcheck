#include "SimulatedDigitalMultimeter.h"

#include <cmath>

InstrumentTimeout::InstrumentTimeout()
    : std::runtime_error("Simulated digital multimeter timed out before a voltage reading was available.")
{
}

SimulatedDigitalMultimeter::SimulatedDigitalMultimeter(const SimulatedBench& bench,
                                                     double voltageOffset,
                                                     bool simulateTimeout)
    : bench_(bench), voltageOffset_(voltageOffset), simulateTimeout_(simulateTimeout)
{
    if (!std::isfinite(voltageOffset_))
    {
        throw std::invalid_argument("Meter voltage offset must be finite.");
    }
}

double SimulatedDigitalMultimeter::measureVoltage() const
{
    if (simulateTimeout_)
    {
        throw InstrumentTimeout();
    }
    return bench_.getOutputVoltage() + voltageOffset_;
}
