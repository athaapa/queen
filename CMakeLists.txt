cmake_minimum_required(VERSION 3.14)

# Force cross-compilation settings before the project() call
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER x86_64-elf-gcc)
set(CMAKE_CXX_COMPILER x86_64-elf-g++)
set(CMAKE_ASM_COMPILER x86_64-elf-gcc)
set(CMAKE_OSX_ARCHITECTURES "")
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(queen LANGUAGES CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(XORRISO_EXECUTABLE xorriso)
find_program(QEMU_SYSTEM_X86_64_EXECUTABLE qemu-system-x86_64)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
    find_package(Threads REQUIRED)

    add_executable(timing_benchmark src/tests/timing_benchmark.cpp)
    target_compile_options(timing_benchmark PRIVATE -O3 -Wall -Wextra -Wpedantic)
    target_compile_definitions(timing_benchmark PRIVATE _GNU_SOURCE)

    add_executable(spsc_baseline src/tests/spsc_baseline.cpp)
    target_compile_options(spsc_baseline PRIVATE -O3 -Wall -Wextra -Wpedantic)
    target_compile_definitions(spsc_baseline PRIVATE _GNU_SOURCE)
    target_link_libraries(spsc_baseline PRIVATE Threads::Threads)
endif()

add_executable(queen_kernel
    src/kernel/main.cpp
    src/kernel/serial.cpp
    src/kernel/panic.cpp
    src/kernel/limine_requests.cpp
    src/kernel/exceptions.cpp
    src/kernel/gdt.cpp
    src/kernel/gdt_flush.S
    src/kernel/idt.cpp
    src/kernel/interrupts.S
    src/kernel/pic.cpp
    src/kernel/memory.cpp
    src/kernel/paging.cpp
    src/kernel/switch_stack.S
    src/kernel/load_cr3.S
    src/kernel/mathutil.cpp
    src/kernel/kernel_main.cpp
    src/kernel/benchmark.cpp
    src/kernel/event_loop.cpp
    src/kernel/mathutil.cpp
    src/kernel/framebuffer.cpp
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

set(LIMINE_DIR ${CMAKE_SOURCE_DIR}/third_party/limine)
set(ISO_ROOT ${CMAKE_BINARY_DIR}/iso_root)
set(QUEEN_ISO ${CMAKE_BINARY_DIR}/queen.iso)

if(XORRISO_EXECUTABLE)
    add_custom_command(
        OUTPUT ${QUEEN_ISO}
        DEPENDS
            queen_kernel
            ${CMAKE_SOURCE_DIR}/src/boot/limine.conf
            ${CMAKE_SOURCE_DIR}/linker.ld
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${ISO_ROOT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_ROOT}/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:queen_kernel> ${ISO_ROOT}/queen.elf
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/boot/limine.conf ${ISO_ROOT}/limine.conf
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios.sys ${ISO_ROOT}/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios-cd.bin ${ISO_ROOT}/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-uefi-cd.bin ${ISO_ROOT}/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/BOOTX64.EFI ${ISO_ROOT}/EFI/BOOT/BOOTX64.EFI
        COMMAND ${XORRISO_EXECUTABLE}
            -as mkisofs
            -b limine-bios-cd.bin
            -no-emul-boot
            -boot-load-size 4
            -boot-info-table
            --efi-boot limine-uefi-cd.bin
            -efi-boot-part
            --efi-boot-image
            --protective-msdos-label
            ${ISO_ROOT}
            -o ${QUEEN_ISO}
        COMMAND ${CMAKE_MAKE_PROGRAM} -C ${LIMINE_DIR}
        COMMAND ${LIMINE_DIR}/limine bios-install ${QUEEN_ISO}
        COMMENT "Building bootable Queen ISO"
        VERBATIM
    )

    add_custom_target(queen_iso DEPENDS ${QUEEN_ISO})
else()
    message(STATUS "Skipping queen_iso target: xorriso not found")
endif()

if(QEMU_SYSTEM_X86_64_EXECUTABLE AND XORRISO_EXECUTABLE)
    add_custom_target(run_queen
        COMMAND ${QEMU_SYSTEM_X86_64_EXECUTABLE}
            -cdrom ${QUEEN_ISO}
            -serial stdio
            -display none
            -no-reboot
            -no-shutdown
        DEPENDS queen_iso
        USES_TERMINAL
        COMMENT "Running Queen in QEMU"
        VERBATIM
    )

    add_custom_target(run_queen_gui
        COMMAND ${QEMU_SYSTEM_X86_64_EXECUTABLE}
            -cdrom ${QUEEN_ISO}
            -serial stdio
            -no-reboot
            -no-shutdown
        DEPENDS queen_iso
        USES_TERMINAL
        COMMENT "Running Queen in QEMU with framebuffer display"
        VERBATIM
    )
elseif(NOT QEMU_SYSTEM_X86_64_EXECUTABLE)
    message(STATUS "Skipping run_queen target: qemu-system-x86_64 not found")
endif()
