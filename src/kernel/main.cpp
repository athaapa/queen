#include "gdt.hpp"
#include "serial.hpp"
#include "time.hpp"
#include <stdint.h>

extern "C" void kernel_main() {
    queen::serial::init();
    queen::gdt::init();

    uint64_t start = queen::read_tsc();
    queen::serial::write("queen booted\n");
    uint64_t end = queen::read_tsc();

    uint64_t delta = end - start;
    queen::serial::write_hex(delta);
    queen::serial::write("\n");

    for (;;) {
        asm volatile("hlt");
    }
}
