#ifndef BENCHCHECK_SIMULATED_DIGITAL_MULTIMETER_H
#define BENCHCHECK_SIMULATED_DIGITAL_MULTIMETER_H

#include "SimulatedBench.h"

#include <stdexcept>

class InstrumentTimeout : public std::runtime_error
{
public:
    InstrumentTimeout();
};

class SimulatedDigitalMultimeter
{
public:
    // The bench must outlive the meter. The optional reading offset and
    // immediate timeout are deterministic fault injection, not electrical effects.
    explicit SimulatedDigitalMultimeter(const SimulatedBench& bench,
                                       double voltageOffset = 0.0,
                                       bool simulateTimeout = false);
    double measureVoltage() const;

private:
    const SimulatedBench& bench_;
    double voltageOffset_;
    bool simulateTimeout_;
};

#endif
