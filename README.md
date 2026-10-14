# BenchCheck

BenchCheck is a C++ electronics test-bench simulator. All instruments are simulated;
no physical instruments or other hardware are controlled.

The C++17/CMake project includes a simulated DC power supply. The `benchcheck`
executable still prints its startup message. A load, multimeter, test runner,
full test-configuration validation, fault injection, safe shutdown orchestration,
pass/fail reports, a testing framework, and CI are future work.

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

The executable prints this line and exits successfully:

```text
BenchCheck — simulated electronics test bench
```

On Windows, use the same configure and build commands, then run
`.\build\Debug\benchcheck.exe` with a Visual Studio (multi-configuration)
generator, or `.\build\benchcheck.exe` with a single-configuration generator
such as Ninja. Multi-configuration generators select Debug with `--config Debug`
rather than `CMAKE_BUILD_TYPE`.

Build output stays in `build/`, which Git ignores. To check a clean build without
removing an existing build, use a new directory such as `build-clean` in both
CMake commands and run the executable from that directory.

## CLion

Use the standalone CLion app. Open the repository root as a CMake project,
confirm the compiler, CMake, and build tool under **Settings > Build, Execution,
Deployment > Toolchains**, and select a Debug CMake profile. If no targets appear,
right-click `CMakeLists.txt` and choose **Load CMake Project**. Select `benchcheck`
to build and run the startup executable.

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
The current limit is stored and can be read back; current flow and current-limit
behavior require a load model and are not simulated here. Even with a 0 A limit,
the enabled output voltage follows the configured voltage in this simple model.

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
This only validates instrument settings; full test-configuration validation
belongs to issue #5.

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

## Project layout

| Path | Purpose |
| --- | --- |
| `src/` | Executable and simulated power supply source |
| `include/` | Public simulated instrument headers |
| `tests/` | Standalone verification program; automated test suite planned for issue #8 |
| `docs/` | Development setup documentation |
| `sample-reports/` | Reserved for future sample reports |

Empty directories contain `.gitkeep` placeholders so they are included in Git.
