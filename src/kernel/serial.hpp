#pragma once

namespace queen {
    namespace serial {
        void init();
        void write(const char* s);
        void write_char(char c);
    }
}