#include "SimulatedPowerSupply.h"

#include <cmath>
#include <stdexcept>

void SimulatedPowerSupply::setVoltage(double volts)
{
    if (!std::isfinite(volts) || volts < kMinVoltage || volts > kMaxVoltage)
    {
        throw std::invalid_argument("Voltage must be finite and between 0 and 30 V.");
    }

    configuredVoltage_ = volts;
}

void SimulatedPowerSupply::setCurrentLimit(double amperes)
{
    if (!std::isfinite(amperes) || amperes < kMinCurrentLimit || amperes > kMaxCurrentLimit)
    {
        throw std::invalid_argument("Current limit must be finite and between 0 and 5 A.");
    }

    currentLimit_ = amperes;
}

void SimulatedPowerSupply::enableOutput()
{
    outputEnabled_ = true;
}

void SimulatedPowerSupply::disableOutput()
{
    outputEnabled_ = false;
}

double SimulatedPowerSupply::getConfiguredVoltage() const
{
    return configuredVoltage_;
}

double SimulatedPowerSupply::getCurrentLimit() const
{
    return currentLimit_;
}

bool SimulatedPowerSupply::isOutputEnabled() const
{
    return outputEnabled_;
}

double SimulatedPowerSupply::getOutputVoltage() const
{
    return outputEnabled_ ? configuredVoltage_ : 0.0;
}
