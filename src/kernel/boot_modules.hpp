#pragma once

#include <stdint.h>

namespace queen {
    namespace boot_modules {
        struct Module {
            void* address;
            uint64_t size;
        };

        void capture();

        const Module& program();

    }
}
