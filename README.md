# BenchCheck

BenchCheck is a C++ electronics test-bench simulator. All instruments are simulated;
no physical instruments or other hardware are controlled.

The C++17/CMake project connects a simulated DC power supply, constant-current
electronic load, and digital multimeter through a shared bench model. The
`benchcheck` executable validates settings, runs a voltage test, shuts down the
bench, prints PASS/FAIL/ERROR, and saves a Markdown report. Normal, undervoltage,
overvoltage, and timeout scenarios are deterministic and require no hardware.
This project demonstrates software simulation, not hands-on lab experience.

Issues #3–#6 are implemented here. Further shutdown verification, a testing
framework with CTest integration, CI, and the final requirement-coverage and
sample-report documentation remain in the later issues.

## Prerequisites

- CMake 3.16 or newer, available as `cmake` on your terminal's `PATH`.
  Install it from the [official CMake downloads](https://cmake.org/download/).
- A compiler with C++17 support: Apple Clang/Clang, GCC, or MSVC.
- A build tool supported by your CMake generator, such as Make, Ninja, or MSBuild.

On macOS, install Apple's Command Line Tools with `xcode-select --install` if
needed; they provide Apple Clang and Make. On Linux, install your distribution's
C++ compiler and Make or Ninja packages. On Windows, install Visual Studio Build
Tools with **Desktop development with C++**, and use its developer terminal.

Check `cmake --version` and your compiler version (`c++ --version` on macOS/Linux
or `cl` in a Visual Studio developer terminal) before configuring.

## Configure, build, and run

From the repository root on macOS or Linux:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
./build/benchcheck
```

The default test configures 5 V, a 0.5 A supply current limit, and a 0.1 A load.
It accepts measured voltages from 4.9 through 5.1 V, including both endpoints.
The executable prints a summary and exits with code 0:

```text
BenchCheck — simulated electronics test bench
Test result: PASS (normal)
Reason: Measured voltage is within the inclusive acceptance limits.
Measured voltage: 5 V; inclusive limits: 4.9 to 5.1 V
Final state: load disabled, supply disabled
Report: benchcheck-report.md
```

The report is written in the current working directory. The default
`benchcheck-report.md` at the repository root is ignored by Git. Use `--report`
to choose another path; its parent directory must exist. An existing report at
that path is replaced. Reports include the test name, scenario, requested
settings and units, inclusive limits, measurement (or explicitly unavailable),
result, reason, simulated instruments, and final enabled/disabled state.

On Windows, use the same configure and build commands, then run
`.\build\Debug\benchcheck.exe` with a Visual Studio (multi-configuration)
generator, or `.\build\benchcheck.exe` with a single-configuration generator
such as Ninja. Multi-configuration generators select Debug with `--config Debug`
rather than `CMAKE_BUILD_TYPE`.

The standard library must support C++17 filesystem paths and floating-point
`std::to_chars`. Apple Clang 21 with CLion's bundled CMake was used for local
verification. Other toolchains have not yet been verified by CI.

Build output stays in `build/`, which Git ignores. To check a clean build without
removing an existing build, use a new directory such as `build-clean` in both
CMake commands and run the executable from that directory.

## CLion

Use the standalone CLion app. Open the repository root as a CMake project,
confirm the compiler, CMake, and build tool under **Settings > Build, Execution,
Deployment > Toolchains**, and select a Debug CMake profile. If no targets appear,
right-click `CMakeLists.txt` and choose **Load CMake Project**. Select `benchcheck`
to build and run the default voltage test. To select a scenario or report path,
set **Program arguments** in the `benchcheck` run configuration, for example
`--scenario timeout --report timeout.md`.

The [previously verified CLion workflow](docs/intellij-idea.md#verified-clion-workflow)
is recorded in the original setup guide. Its IntelliJ IDEA plugin instructions
are historical; those plugins are not needed for standalone CLion. CLion can use
its bundled CMake even when `cmake` is not on the terminal's `PATH`.

## Simulated power supply

`SimulatedPowerSupply` is declared in `include/SimulatedPowerSupply.h`. It stores
two settings and an output-enabled flag; it never connects to hardware.

| Setting/state | Supported values | Initial value |
| --- | --- | --- |
| Configured voltage | Finite values from 0 to 30 V, inclusive | 0 V |
| Current limit | Finite values from 0 to 5 A, inclusive | 0 A |
| Output enabled | `true` or `false` | `false` |
| Actual output voltage | Configured voltage when enabled; 0 V when disabled | 0 V |

These ranges are choices for this simulated instrument, not hardware ratings.
The supply stores its current limit; `SimulatedBench` checks active load demand
against it. Reading the standalone supply does not perform this circuit check.
With no load demand, even a 0 A limit leaves the enabled output voltage equal to
the configured voltage in this ideal model.

Configured voltage is the value the supply remembers. Actual output voltage is
what it currently presents at its simulated output. For example, configuring
12 V while disabled remembers 12 V but still produces 0 V. Only `enableOutput()`
enables the output. `disableOutput()` returns the output to 0 V and preserves
both settings, so enabling again restores the configured voltage. Repeated
disable calls are safe. Changing either setting leaves the enabled state alone;
changing voltage while enabled updates the output voltage immediately.

```cpp
#include "SimulatedPowerSupply.h"

SimulatedPowerSupply supply;          // 0 V, 0 A, output disabled
supply.setVoltage(12.0);              // configured: 12 V, actual: 0 V
supply.setCurrentLimit(1.5);           // amperes; does not enable output
supply.enableOutput();                // actual: 12 V
supply.disableOutput();               // actual: 0 V, configured: still 12 V
```

Use `getConfiguredVoltage()`, `getCurrentLimit()`, `isOutputEnabled()`, and
`getOutputVoltage()` to read the state. The getters are `const`, so reading does
not change the supply. Setters reject out-of-range values, infinity, and NaN
(not-a-number) by throwing `std::invalid_argument`. Validation happens before
assignment, so every setting and the output state remain unchanged on failure.
Callers can catch this exception (include `<stdexcept>`) to handle invalid input.
These setters validate individual instrument settings. The test runner also
validates the complete configuration before configuring or enabling instruments.

## Shared bench, load, and digital multimeter

`SimulatedBench` owns the supply and load. `SimulatedDigitalMultimeter` reads that
bench by reference; the bench must outlive the meter. Its normal reading always
follows live supply state rather than a fixed test value.

```cpp
#include "SimulatedDigitalMultimeter.h"

SimulatedBench bench;
SimulatedDigitalMultimeter meter(bench);
bench.supply().setVoltage(5.0);
bench.supply().setCurrentLimit(0.5);
bench.load().setCurrent(0.1);
bench.supply().enableOutput();
bench.load().enableInput();
const double voltage = meter.measureVoltage(); // 5 V
const double current = bench.getLoadCurrent(); // 0.1 A
bench.load().disableInput();
bench.supply().disableOutput();
```

The load starts disabled at 0 A and accepts finite settings from 0 to 5 A.
Setting its current never enables it. Disabling it is idempotent and preserves
the setting. Invalid setters throw `std::invalid_argument` without state changes.

The simplified electrical rules are:

- With supply and load enabled, current equals the configured load demand. The
  supply voltage stays at its configured value, including at the current limit.
- Disabling the load makes current zero while leaving the supply voltage alone.
- Disabling the supply makes normal measured voltage and load current zero,
  regardless of the load's enabled state or current setting.
- If both instruments are enabled and demand exceeds the supply current limit,
  bench voltage/current reads throw `std::runtime_error`. This simulator reports
  overcurrent instead of modeling voltage droop or a constant-current transition.
- These are ideal logical instruments: there is no settling time, noise,
  resistance, power limit, thermal behavior, or physical zero-voltage load model.

## Scenarios and command-line settings

```sh
./build/benchcheck --help
./build/benchcheck --scenario normal --report normal.md
./build/benchcheck --scenario undervoltage --report undervoltage.md
./build/benchcheck --scenario overvoltage --report overvoltage.md
./build/benchcheck --scenario timeout --report timeout.md
./build/benchcheck --voltage 12 --current-limit 2 --load-current 1 --min-voltage 11.9 --max-voltage 12.1 --report twelve-volts.md
```

| Scenario | Default measurement | Result | Exit code |
| --- | --- | --- | --- |
| `normal` | 5 V | PASS | 0 |
| `undervoltage` | 4.7 V | FAIL | 1 |
| `overvoltage` | 5.3 V | FAIL | 1 |
| `timeout` | Unavailable | ERROR | 2 |

Voltage faults apply a deterministic offset to the live bench reading. They
move it at least 0.2 V beyond the selected lower/upper limit, even with custom
settings. They model incorrect meter readings, not changes in supply settings;
an injected reading can be below 0 V or above 30 V. Timeout raises a simulated
`InstrumentTimeout` immediately, without sleeping or waiting for hardware.
Identical settings and scenario produce identical results and report contents.

PASS means a completed reading falls within the inclusive limits. FAIL means a
completed reading is outside them, including in `normal` with custom settings.
ERROR means the test could not complete, arguments are invalid, or the report
could not be saved. Missing measurements are stored as `std::nullopt` and shown
as unavailable, never substituted with 0 V. Numeric output preserves enough
precision to distinguish a reading just outside a limit from the limit itself.

The runner disables the load before the supply, including after validation
failure, measurement failure, or timeout. It also disables a previously enabled
bench before starting. Scope-based cleanup covers exceptions during the test.
Reporting happens after shutdown. A report open/write/flush/close failure prints
a clear `ERROR` and returns 2 even if the preceding measurement passed or failed;
that measurement result is still visible in the console.

## Configuration validation

Every numeric setting must be finite: NaN and positive/negative infinity are
rejected. The following ranges are inclusive:

| Option / configuration field | Allowed values | Default |
| --- | --- | --- |
| `--voltage` / `voltage` | 0–30 V | 5 V |
| `--current-limit` / `currentLimit` | 0–5 A | 0.5 A |
| `--load-current` / `loadCurrent` | 0–5 A and no greater than `currentLimit` | 0.1 A |
| `--min-voltage` / `minimumVoltage` | 0–30 V and no greater than `maximumVoltage` | 4.9 V |
| `--max-voltage` / `maximumVoltage` | 0–30 V | 5.1 V |

Equal minimum/maximum limits, zero settings, and load current equal to the
supply current limit are valid. The nominal supply voltage may be outside the
acceptance band; that is a valid configuration whose normal measurement fails.

`runVoltageTest()` validates the entire `TestConfiguration` before changing
settings or enabling either instrument. A rejected configuration returns ERROR
with the invalid field in its reason, no reading, and both instruments disabled.
The CLI writes this result to its report and returns 2. For example:

```sh
./build/benchcheck --load-current 0.6 --report invalid.md
```

This reports `load-current must not exceed current-limit.` Unknown or duplicate
options, missing values, malformed numbers, and unknown scenarios are command-line
errors: they return 2 before creating a bench or report. `--help` returns 0
without running a test or writing a report. Options take separate values (for
example, `--voltage 5`); quote report paths containing spaces.

## Repeatable power supply verification

This small standalone program checks the issue #2 behavior without a testing
framework. It is built only when explicitly requested. After configuring CMake:

```sh
cmake --build build --config Debug --target verify_simulated_power_supply
./build/verify_simulated_power_supply
```

On Windows with a Visual Studio generator, run
`.\build\Debug\verify_simulated_power_supply.exe`; with a single-configuration
generator, run `.\build\verify_simulated_power_supply.exe`. In CLion, reload
CMake and select the `verify_simulated_power_supply` target to build and run it.

Success prints `All simulated power supply checks passed.` and exits with code 0.
A failed check prints a reason and exits with code 1. Checks remain active in
Release builds. The program verifies initial state, configuration without
enabling, enable/disable behavior, repeated disabling, preserved settings when
re-enabled, inclusive range endpoints, and invalid settings without state changes
while both enabled and disabled. It also checks valid updates after rejection.

This is an explicitly run verification program for the current change. The
automated test suite, testing framework, and test integration belong to issue #8;
no CTest tests or CI checks are registered here.

## Repeatable bench and CLI verification

The issue #3–#6 checks follow the same standalone approach and add no downloaded
dependencies. After configuring CMake, run:

```sh
cmake --build build --config Debug --target verify_simulated_power_supply verify_benchcheck verify_cli
./build/verify_simulated_power_supply
./build/verify_benchcheck
```

The `verify_cli` target runs the actual executable with normal and fault scenarios,
custom and invalid settings, malformed arguments, and report paths that fail.
It checks exit codes and generated report content; each process has a five-second
timeout to catch hangs. Its reports stay under `build/cli-verification/`.

`verify_benchcheck` checks live bench measurements, load controls and range
validation, overcurrent errors, all scenario outcomes, deterministic repetitions,
inclusive limits and their adjacent floating-point values, complete configuration
validation, missing readings, shutdown state, report content, and report-open
failure. It exits 0 with a success message or 1 with a failure reason. Checks stay
active in Release builds. The original supply checks remain unchanged.

On Windows with a Visual Studio generator, run the verification executables from
`build\Debug\`; single-configuration generators put them directly in `build\`.
In CLion, reload CMake and select either verification executable or `verify_cli`.
CTest and a third-party testing framework remain scheduled for issue #8.

## Project layout

| Path | Purpose |
| --- | --- |
| `src/` | CLI, simulated instruments, bench, voltage runner, and reporting |
| `include/` | Public instrument, bench, configuration/result, and report headers |
| `tests/` | Standalone C++ and CLI checks; framework/CTest planned for issue #8 |
| `docs/` | Development setup documentation |
| `sample-reports/` | Reserved for future sample reports |

Empty directories contain `.gitkeep` placeholders so they are included in Git.
