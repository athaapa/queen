#include "kernel_main.hpp"
#include "benchmark.hpp"
#include "framebuffer.hpp"
#include "memory.hpp"
#include "serial.hpp"

extern "C" char queen_program_start[];
extern "C" char queen_program_end[];

[[noreturn]] void queen::kernel_runtime_main() {
    queen::memory::seal_boot_allocations();
    queen::serial::write_line("queen memory initialized");
    queen::framebuffer::write("queen booted\n");
    queen::benchmark::run_tsc_overhead();
    queen::benchmark::run_event_loop();

    queen::serial::write("queen_program_start: ");
    queen::serial::write_hex(reinterpret_cast<uint64_t>(queen_program_start));
    queen::serial::write("\n");

    queen::serial::write("queen_program_end: ");
    queen::serial::write_hex(reinterpret_cast<uint64_t>(queen_program_end));
    queen::serial::write("\n");

    queen::serial::write("queen_program_size: ");
    queen::serial::write_hex(reinterpret_cast<uint64_t>(queen_program_end)
        - reinterpret_cast<uint64_t>(queen_program_start));
    queen::serial::write("\n");

    for (;;) {
        asm volatile("hlt");
    }
}
