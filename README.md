# BenchCheck

BenchCheck is a C++ electronics test-bench simulator. All instruments are simulated;
no physical instruments or other hardware are controlled.

The current project is a minimal C++17/CMake starter that prints a startup message.
Instrument simulation, measurement validation, fault injection, safe shutdown,
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

## IntelliJ IDEA

See [the IntelliJ IDEA setup guide](docs/intellij-idea.md) for plugin compatibility,
compiler and CMake configuration, and IDE build/run steps. Published compatibility
has been checked; the guide lists the local IDE checks still requiring manual
verification. Terminal builds do not require an IDE plugin.

## Project layout

| Path | Purpose |
| --- | --- |
| `src/` | Executable source |
| `include/` | Reserved for future project headers |
| `tests/` | Reserved for future tests; no testing framework yet |
| `docs/` | Development setup documentation |
| `sample-reports/` | Reserved for future sample reports |

Empty directories contain `.gitkeep` placeholders so they are included in Git.
