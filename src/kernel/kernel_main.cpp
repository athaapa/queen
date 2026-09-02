#include "kernel_main.hpp"
#include "boot_modules.hpp"
#include "framebuffer.hpp"
#include "memory.hpp"
#include "program.hpp"
#include "serial.hpp"
#include <stdint.h>

extern "C" char queen_program_start[];
extern "C" char queen_program_end[];
extern "C" int64_t queen_event_loop_entry(uint64_t id, uint64_t value);

[[noreturn]] void queen::kernel_runtime_main() {
    queen::memory::seal_boot_allocations();
    queen::serial::write_line("queen memory initialized");
    queen::framebuffer::write("queen booted\n");

    const queen::boot_modules::Module& program_module = queen::boot_modules::program();

    program::Entry entry = reinterpret_cast<program::Entry>(program_module.address);

    uint64_t res = entry(1, 3);
    queen::serial::write_decimal(res);

    // queen::benchmark::run_tsc_overhead();
    // queen::benchmark::run_event_loop();

    for (;;) {
        asm volatile("hlt");
    }
}
