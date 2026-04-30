#include "serial.hpp"
#include <cstdint>

constexpr uint16_t COM1 = 0x3F8;

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void queen::serial::init() {
    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80); // enable divisor latch
    outb(COM1 + 0, 0x03); // divisor low: 38400 baud
    outb(COM1 + 1, 0x00); // divisor high
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static bool can_transmit() { return (inb(COM1 + 5) & 0x20) != 0; }

void queen::serial::write_char(char c) {
    while (!can_transmit()) { }
    outb(COM1, static_cast<uint8_t>(c));
}

void queen::serial::write(const char* s) {
    while (*s != '\0') {
        write_char(*s++);
    }
}