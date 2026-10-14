# IntelliJ IDEA setup

BenchCheck uses C++17 and CMake. All instruments are simulated; no physical
hardware is controlled. This guide covers the starter executable only.

## Compatibility checked on October 12, 2026

The locally installed IDE is IntelliJ IDEA 2026.2.3, build `IU-262.10968.63`, on
Apple Silicon macOS. JetBrains Marketplace's official version metadata lists
these compatible releases for IntelliJ IDEA Pro 2026.2.3:

| JetBrains plugin | Compatible release |
| --- | --- |
| [CLion C and C++](https://plugins.jetbrains.com/plugin/28804-clion-c-and-c-/versions/stable/1171907) | `262.10968.63-mac-arm64` |
| [CMake](https://plugins.jetbrains.com/plugin/28794-cmake/versions/stable/1171906) | `262.10968.63` |
| [Native Build Tools](https://plugins.jetbrains.com/plugin/28796-native-build-tools/versions/stable/1171962) | `262.10968.63` |

These releases declare a supported build range of `262.10968` through `262.*`.
This verifies published compatibility, not successful installation or execution
in this local IDE. Do not assume the same releases support older IDEA versions,
other editions, or another operating system/CPU. Use Marketplace's compatible
release for your exact IDE and platform, and verify any license requirements in
the IDE. Do not blindly install the newest release: a 263.x plugin may require
IDEA 2026.3.

JetBrains' [toolchain guide](https://www.jetbrains.com/help/clion/how-to-create-toolchain-in-clion.html)
and [CMake tutorial](https://www.jetbrains.com/help/clion/quick-cmake-tutorial.html)
describe the native build workflow in CLion. The corresponding IDEA plugin UI
steps below still require local verification; menu availability may differ.

## Install and open

1. Check **Help > About** (or **IntelliJ IDEA > About** on macOS) for the IDE build.
2. In **Settings > Plugins > Marketplace**, install JetBrains' **CLion C and C++**
   plugin for that build and platform. Ensure **CMake** and **Native Build Tools**
   are installed and enabled, accepting required plugin dependencies. Restart
   when prompted. A syntax-highlighting-only C++ plugin is insufficient for the
   native CMake build/run workflow.
3. Choose **File > Open** and select the repository root containing
   `CMakeLists.txt`. Load it as a CMake project if prompted.

## Configure the compiler and CMake

Install the [terminal prerequisites](../README.md#prerequisites) first. A Java JDK
does not provide the C++ compiler or CMake.

1. Open **Settings > Build, Execution, Deployment > Toolchains**. Select or add
   the local toolchain: System on macOS/Linux, or Visual Studio on Windows.
2. Check the C++ compiler. On this Mac, `/usr/bin/clang++` uses the installed Apple
   Command Line Tools (Apple Clang 21). On Linux use Clang or GCC; on Windows use
   the compiler from the Visual Studio C++ workload.
3. Select CMake 3.16 or newer. Use a bundled CMake only if the plugins actually
   provide it; otherwise browse to your installed CMake executable. On macOS the
   official app distribution contains `CMake.app/Contents/bin/cmake`. A terminal
   needs that directory on `PATH` to use the README commands.
4. Select a working build tool: bundled Ninja if available, otherwise installed
   Ninja or Make (or the appropriate Visual Studio build tools). Resolve any
   toolchain detection errors before continuing.
5. Under **Settings > Build, Execution, Deployment > CMake**, create or select a
   **Debug** profile using this toolchain and `cmake-build-debug` as its build
   directory. The project declares C++17; no extra language-standard option is
   needed. Apply the settings and reload CMake.

## Build and run

1. Confirm CMake configuration succeeds and discovers the `benchcheck` target.
2. Select its **CMake Application** run configuration. If none is created
   automatically, add one with target and executable set to `benchcheck`, using
   the Debug profile and a build step before launch.
3. Build the target using **Build Project**, then run the `benchcheck`
   configuration.
4. Confirm the Run console shows `BenchCheck — simulated electronics test bench`
   and exit code 0.

If native CMake menus are missing, recheck plugin compatibility, dependencies,
and enabled state. You can still open the repository for editing and run the
README configure/build/run commands in **View > Tool Windows > Terminal** after
installing the terminal prerequisites. That fallback does not verify native IDE
integration.

## Manual verification still needed

- [ ] Install/enable compatible plugins and dependencies; confirm license/access
      requirements and restart successfully.
- [ ] Confirm the Toolchains and CMake settings are present and detect the
      compiler, CMake, and build tool without errors.
- [ ] Open the root as a CMake project and load the Debug profile successfully.
- [ ] Build and run `benchcheck` through the IDE and verify the exact output and
      exit code 0.

The Marketplace compatibility check and terminal verification do not complete
these GUI checks. Personal `.idea/`, `*.iml`, and `CMakeUserPresets.json` settings
and `cmake-build-*` output are ignored; keep machine-specific paths out of Git.

## Verified CLion workflow

Verified on October 14, 2026:

1. Open the existing `benchcheck` repository folder in CLion.
2. Under Settings > Build, Execution, Deployment > Toolchains,
   confirm CMake, Ninja, and the C++ compiler are detected.
3. Under CMake, enable the Debug profile using the default
   toolchain and `cmake-build-debug` build directory.
4. If the toolbar shows "Add Configuration", right-click the
   root `CMakeLists.txt` and select "Load CMake Project".
5. Select `benchcheck` and click Run.

The executable printed:

    BenchCheck — simulated electronics test bench

The process exited with code 0.

CLion's CMake import, build, and run are verified.
The separate IntelliJ IDEA plugin workflow remains unverified.