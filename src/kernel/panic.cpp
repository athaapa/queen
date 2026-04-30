#include "panic.hpp"
#include "serial.hpp"

[[noreturn]] void queen::panic(const char* message) {
    serial::write("panic: ");
    serial::write(message);
    serial::write("\n");

    for (;;) {
        asm volatile("cli; hlt");
    }
}