#include "SimulatedBench.h"

#include <stdexcept>

SimulatedPowerSupply& SimulatedBench::supply() { return supply_; }
const SimulatedPowerSupply& SimulatedBench::supply() const { return supply_; }
SimulatedLoad& SimulatedBench::load() { return load_; }
const SimulatedLoad& SimulatedBench::load() const { return load_; }

double SimulatedBench::getOutputVoltage() const
{
    // Validate the same circuit state used by current measurements.
    (void)getLoadCurrent();
    return supply_.getOutputVoltage();
}

double SimulatedBench::getLoadCurrent() const
{
    if (!supply_.isOutputEnabled() || !load_.isInputEnabled())
    {
        return 0.0;
    }
    if (load_.getConfiguredCurrent() > supply_.getCurrentLimit())
    {
        throw std::runtime_error("Load current exceeds the supply current limit.");
    }
    return load_.getConfiguredCurrent();
}
