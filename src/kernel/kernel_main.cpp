#include "kernel_main.hpp"
#include "benchmark.hpp"
#include "memory.hpp"
#include "serial.hpp"

[[noreturn]] void queen::kernel_runtime_main() {
    queen::memory::seal_boot_allocations();
    queen::serial::write_line("queen memory initialized");
    queen::benchmark::run_tsc_overhead();
    queen::benchmark::run_event_loop();

    for (;;) {
        asm volatile("hlt");
    }
}
