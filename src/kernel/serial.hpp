#pragma once
#include <stdint.h>

namespace queen {
    namespace serial {
        void init();

        // Writes a string to COM1
        void write(const char* s);

        // Writes a character to COM1
        void write_char(char c);

        // Writes value in hexadecimal to COM1
        void write_hex(uint64_t value);
    }
}
