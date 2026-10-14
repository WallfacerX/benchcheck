#ifndef BENCHCHECK_SIMULATED_BENCH_H
#define BENCHCHECK_SIMULATED_BENCH_H

#include "SimulatedLoad.h"
#include "SimulatedPowerSupply.h"

// Owns the instruments whose live state determines voltage and current.
class SimulatedBench
{
public:
    SimulatedPowerSupply& supply();
    const SimulatedPowerSupply& supply() const;
    SimulatedLoad& load();
    const SimulatedLoad& load() const;

    // Throws std::runtime_error if an enabled load demands more than the
    // enabled supply's current limit. No voltage-droop model is simulated.
    double getOutputVoltage() const;
    double getLoadCurrent() const;

private:
    SimulatedPowerSupply supply_;
    SimulatedLoad load_;
};

#endif
