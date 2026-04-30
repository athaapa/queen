#include "serial.hpp"
#include <cstdint>

static inline void debugcon_write_char(char c) {
    asm volatile("outb %0, %1" : : "a"(static_cast<uint8_t>(c)), "Nd"(0xE9));
}

static void debugcon_write(const char* s) {
    while (*s != '\0') {
        debugcon_write_char(*s++);
    }
}

extern "C" void kernel_main() {
    debugcon_write("entered kernel_main\n");

    queen::serial::init();
    queen::serial::write("queen booted\n");

    for (;;) {
        asm volatile("hlt");
    }
}