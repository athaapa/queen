#pragma once
#include <stdint.h>

namespace queen {
    namespace idt {
        struct __attribute__((packed)) IDTEntry {
            uint16_t offset_1; // offset bits 0..15
            uint16_t selector; // a code segment selector in GDT or LDT
            uint8_t ist; // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
            uint8_t type_attributes; // gate type, dpl, and p fields
            uint16_t offset_2; // offset bits 16..31
            uint32_t offset_3; // offset bits 32..63
            uint32_t reserved; // reserved
        };

        struct __attribute__((packed)) IDTR {
            uint16_t size;
            uint64_t offset;
        };

        void init();
    }
}
