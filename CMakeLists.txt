cmake_minimum_required(VERSION 3.16)

project(benchcheck LANGUAGES CXX)

add_library(simulated_power_supply src/SimulatedPowerSupply.cpp)
target_include_directories(simulated_power_supply PUBLIC include)
target_compile_features(simulated_power_supply PUBLIC cxx_std_17)
set_target_properties(simulated_power_supply PROPERTIES CXX_EXTENSIONS NO)

add_executable(benchcheck src/main.cpp)
target_link_libraries(benchcheck PRIVATE simulated_power_supply)
set_target_properties(benchcheck PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Run this small verification program explicitly; issue #8 adds the test suite.
add_executable(verify_simulated_power_supply EXCLUDE_FROM_ALL
    tests/verify_simulated_power_supply.cpp
)
target_link_libraries(verify_simulated_power_supply PRIVATE simulated_power_supply)
set_target_properties(verify_simulated_power_supply PROPERTIES CXX_EXTENSIONS NO)
