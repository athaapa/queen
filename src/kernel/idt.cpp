#include "idt.hpp"

queen::idt::IDTEntry idt_entries[256];
queen::idt::IDTR idtr;

constexpr uint32_t OFFSET_1_MASK = 0xFFFF;
constexpr uint32_t OFFSET_2_MASK = 0xFFFF;
constexpr uint32_t OFFSET_3_MASK = 0xFFFFFFFF;

constexpr uint8_t INTERRUPT_GATE = 0x8E;
constexpr uint8_t TRAP_GATE = 0x8F;

extern "C" void isr0();
extern "C" void isr32();
extern "C" void isr14();

static void set_idt_gate(int interrupt_number, uint64_t handler_address);

void queen::idt::init() {
    idtr.size = sizeof(idt_entries) - 1;
    idtr.offset = (uint64_t)&idt_entries;

    set_idt_gate(0, (uint64_t)isr0);
    set_idt_gate(32, (uint64_t)isr32);
    set_idt_gate(14, (uint64_t)isr14);

    asm volatile("lidt %0" : : "m"(idtr));
}

static void set_idt_gate(int interrupt_number, uint64_t handler_address) {
    queen::idt::IDTEntry& entry = idt_entries[interrupt_number];

    entry.offset_1 = static_cast<uint16_t>(handler_address & OFFSET_1_MASK);
    entry.offset_2 = static_cast<uint16_t>((handler_address >> 16) & OFFSET_2_MASK);
    entry.offset_3 = static_cast<uint32_t>((handler_address >> 32) & OFFSET_3_MASK);

    entry.selector = 0x08; // Kernel Code segment
    entry.type_attributes = INTERRUPT_GATE;
}
