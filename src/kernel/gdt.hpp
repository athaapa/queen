#pragma once
#include <stdint.h>

namespace queen {
    namespace gdt {
        struct __attribute__((packed)) GDTEntry {
            uint16_t limit_low;
            uint16_t base_low;
            uint8_t base_middle;
            uint8_t access;
            uint8_t limit_high_and_flags;
            uint8_t base_high;
        };

        struct __attribute__((packed)) GDTR {
            uint16_t limit;
            uint64_t base;
        };

        void init();
    }
}
