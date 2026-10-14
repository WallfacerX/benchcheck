#include "SimulatedLoad.h"

#include <cmath>
#include <stdexcept>

void SimulatedLoad::setCurrent(double amperes)
{
    if (!std::isfinite(amperes) || amperes < kMinCurrent || amperes > kMaxCurrent)
    {
        throw std::invalid_argument("Load current must be finite and between 0 and 5 A.");
    }
    configuredCurrent_ = amperes;
}

void SimulatedLoad::enableInput()
{
    inputEnabled_ = true;
}

void SimulatedLoad::disableInput() noexcept
{
    inputEnabled_ = false;
}

double SimulatedLoad::getConfiguredCurrent() const
{
    return configuredCurrent_;
}

bool SimulatedLoad::isInputEnabled() const
{
    return inputEnabled_;
}
