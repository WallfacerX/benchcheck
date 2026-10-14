cmake_minimum_required(VERSION 3.16)

project(benchcheck LANGUAGES CXX)

add_executable(benchcheck src/main.cpp)
set_target_properties(benchcheck PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
