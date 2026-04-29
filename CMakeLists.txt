cmake_minimum_required(VERSION 3.14)
project(queen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(timing_benchmark src/tests/timing_benchmark.cpp)
target_compile_options(timing_benchmark PRIVATE -O3 -Wall -Wextra -Wpedantic)
target_compile_definitions(timing_benchmark PRIVATE _GNU_SOURCE)