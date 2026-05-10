#include "framebuffer.hpp"
#include "gdt.hpp"
#include "idt.hpp"
#include "memory.hpp"
#include "paging.hpp"
#include "pic.hpp"
#include "serial.hpp"

extern "C" void kernel_main() {
    queen::serial::init();
    queen::gdt::init();
    queen::idt::init();
    queen::pic::init();
    queen::memory::init();
    queen::framebuffer::init();
    queen::paging::init();

    queen::serial::write("queen booted\n");
    queen::paging::activate();

    for (;;) {
        asm volatile("hlt");
    }
}
