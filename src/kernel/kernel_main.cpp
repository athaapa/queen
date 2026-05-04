#include "kernel_main.hpp"
#include "memory.hpp"
#include "serial.hpp"

[[noreturn]] void queen::kernel_runtime_main() {
    queen::memory::seal_boot_allocations();
    queen::serial::write_line("queen memory initialized");

    for (;;) {
        asm volatile("hlt");
    }
}
