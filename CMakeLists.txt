cmake_minimum_required(VERSION 3.14)
project(queen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

add_executable(timing_benchmark src/tests/timing_benchmark.cpp)
target_compile_options(timing_benchmark PRIVATE -O3 -Wall -Wextra -Wpedantic)
target_compile_definitions(timing_benchmark PRIVATE _GNU_SOURCE)

add_executable(spsc_baseline src/tests/spsc_baseline.cpp)
target_compile_options(spsc_baseline PRIVATE -O3 -Wall -Wextra -Wpedantic)
target_compile_definitions(spsc_baseline PRIVATE _GNU_SOURCE)
target_link_libraries(spsc_baseline PRIVATE Threads::Threads)