#include "gdt.hpp"

queen::gdt::GDTEntry gdt_entries[3];
queen::gdt::GDTR gdtr;

constexpr uint8_t KERNEL_CODE_ACCESS = 0x9A;
constexpr uint8_t KERNEL_CODE_FLAGS_AND_LIMIT = 0xA << 4;

constexpr uint8_t KERNEL_DATA_ACCESS = 0x92;
constexpr uint8_t KERNEL_DATA_FLAGS_AND_LIMIT = 0xC << 4;

extern "C" void load_gdt(uint64_t gdtr_address);

void queen::gdt::init() {
    // Null descriptor segment
    gdt_entries[0] = { 0, 0, 0, 0, 0, 0 };

    // Kernel code segment
    gdt_entries[1] = {
        0,
        0,
        0,
        KERNEL_CODE_ACCESS,
        KERNEL_CODE_FLAGS_AND_LIMIT,
        0,
    };

    // Kernel data segment
    gdt_entries[2] = {
        0,
        0,
        0,
        KERNEL_DATA_ACCESS,
        KERNEL_DATA_FLAGS_AND_LIMIT,
        0,
    };

    gdtr.base = (uint64_t)&gdt_entries;
    gdtr.limit = sizeof(gdt_entries) - 1;

    load_gdt((uint64_t)&gdtr);
}
