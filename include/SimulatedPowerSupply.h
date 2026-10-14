#ifndef BENCHCHECK_SIMULATED_POWER_SUPPLY_H
#define BENCHCHECK_SIMULATED_POWER_SUPPLY_H

// An ideal, simulated DC power supply. This class never accesses hardware.
class SimulatedPowerSupply
{
public:
    // Supported settings are finite values in these inclusive ranges.
    static constexpr double kMinVoltage = 0.0;      // volts
    static constexpr double kMaxVoltage = 30.0;     // volts
    static constexpr double kMinCurrentLimit = 0.0; // amperes
    static constexpr double kMaxCurrentLimit = 5.0; // amperes

    // A new supply is configured for 0 V and 0 A, with output disabled.
    SimulatedPowerSupply() = default;

    // Invalid settings throw std::invalid_argument without changing any state.
    // Setting either value leaves the output enabled/disabled state unchanged.
    void setVoltage(double volts);
    void setCurrentLimit(double amperes);

    void enableOutput();
    // Keeps both settings; calling this repeatedly is safe.
    void disableOutput() noexcept;

    double getConfiguredVoltage() const;
    double getCurrentLimit() const;
    bool isOutputEnabled() const;

    // Actual simulated voltage: configured voltage when enabled, otherwise 0 V.
    double getOutputVoltage() const;

private:
    double configuredVoltage_ = 0.0;
    // SimulatedBench checks load demand against this setting.
    double currentLimit_ = 0.0;
    bool outputEnabled_ = false;
};

#endif
