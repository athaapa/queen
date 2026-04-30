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

add_executable(queen_kernel
    src/kernel/main.cpp
    src/kernel/serial.cpp
    src/kernel/panic.cpp
    src/kernel/limine_requests.cpp
)

set_target_properties(queen_kernel PROPERTIES
    OUTPUT_NAME queen
    SUFFIX ".elf"
)

target_compile_options(queen_kernel PRIVATE
    -ffreestanding
    -fno-exceptions
    -fno-rtti
    -fno-stack-protector
    -fno-pic
    -fno-pie
    -mno-red-zone
    -Wall
    -Wextra
    -Wpedantic
    -mcmodel=kernel
)

target_link_options(queen_kernel PRIVATE
    -nostdlib
    -no-pie
    -static
    -T ${CMAKE_SOURCE_DIR}/linker.ld
    -Wl,-z,max-page-size=0x1000
)