#include "serial.hpp"

#include <stdint.h>

extern "C" void panic() { queen::serial::write("\nFATAL: DIVIDE BY ZERO EXCEPTION\n"); }

extern "C" void tick() { queen::serial::write("tick\n"); }

extern "C" void page_fault_handler(uint64_t fault_addr, uint64_t error_code) {
    queen::serial::write_line("PAGE FAULT");
    queen::serial::write("fault_addr: ");
    queen::serial::write_hex(fault_addr);
    queen::serial::write("\n");
    queen::serial::write("error_code: ");
    queen::serial::write_hex(error_code);
    queen::serial::write("\n");

    for (;;) {
        asm volatile("hlt");
    }
}
