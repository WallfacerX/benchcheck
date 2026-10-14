#ifndef BENCHCHECK_SIMULATED_LOAD_H
#define BENCHCHECK_SIMULATED_LOAD_H

// An ideal constant-current electronic load. Never accesses hardware.
class SimulatedLoad
{
public:
    static constexpr double kMinCurrent = 0.0; // amperes
    static constexpr double kMaxCurrent = 5.0; // amperes

    // Rejects non-finite/out-of-range settings without changing state.
    void setCurrent(double amperes);
    void enableInput();
    void disableInput() noexcept;

    double getConfiguredCurrent() const;
    bool isInputEnabled() const;

private:
    double configuredCurrent_ = 0.0;
    bool inputEnabled_ = false;
};

#endif
