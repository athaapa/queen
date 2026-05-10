#pragma once
#include <stdint.h>

namespace queen {
    namespace framebuffer {
        void init();
        uint64_t virtual_base();
        uint64_t size_bytes();
        bool available();
        void fill(uint32_t color);
        void put_char(char c);
        void write(const char* c);
        void write_hex(uint64_t value);
        void write_decimal(uint64_t value);
    }
}
