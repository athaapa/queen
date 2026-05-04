#pragma once
#include <stdint.h>

namespace queen {
    // Why is this inline?
    inline uint64_t read_tsc() {
        uint64_t value = 0;
        asm volatile("rdtsc");

        uint32_t eax_value, edx_value;
        asm volatile("movl %%eax, %0" : "=r"(eax_value));
        asm volatile("movl %%edx, %0" : "=r"(edx_value));

        value |= eax_value;
        value |= (edx_value << 31);

        return value;
    }
}
