#include "serial.hpp"

extern "C" void kernel_main() {
    queen::serial::init();
    queen::serial::write("queen booted\n");

    for (;;) {
        asm volatile("hlt");
    }
}