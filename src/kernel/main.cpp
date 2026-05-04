#include "gdt.hpp"
#include "idt.hpp"
#include "limine.h"
#include "memory.hpp"
#include "paging.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "time.hpp"

#include <stdint.h>

extern "C" void panic() { queen::serial::write("\nFATAL: DIVIDE BY ZERO EXCEPTION\n"); }
extern "C" void tick() { queen::serial::write("tick\n"); }
extern "C" void switch_stack(uint64_t stack_top, uint64_t pml4_phys);

extern "C" void kernel_main() {
    queen::serial::init();
    queen::gdt::init();
    queen::idt::init();
    queen::pic::init();
    queen::memory::init();
    queen::paging::init();

    queen::serial::write("queen booted\n");

    // asm volatile("sti"); // Q: What does this do and why is it necessary?
    switch_stack(queen::paging::get_kernel_stack_top(), queen::paging::get_pml4_phys());

    for (;;) {
        asm volatile("hlt");
    }
}
