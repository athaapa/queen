#include "pic.hpp"

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value);
static inline uint8_t inb(uint16_t port);

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// TODO: Clean this up
void queen::pic::init() {
    // Send the initialization command to both PICs
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // Tell Master PIC to start at IDT entry 32 (0x20)
    outb(0x21, 0x20);
    // Tell Slave PIC to start at IDT entry 32 (0x20)
    outb(0xA1, 0x28);

    // Tell them how they are wired together
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // Set the mode to 8086/88
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // MASK everything except timer (IRQ0)
    outb(0x21, 0xFE);
    outb(0xA1, 0xFF);
}
