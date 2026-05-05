#pragma once
#include <stdint.h>

namespace queen {
    inline uint64_t read_tsc() {
        uint32_t lo = 0, hi = 0;
        asm volatile("rdtsc" : "=a"(lo), "=d"(hi) : : "memory");

        return (static_cast<uint64_t>(hi) << 32) | lo;
    }

    inline uint64_t read_tsc_ordered() {
        uint32_t lo = 0, hi = 0;
        asm volatile("lfence\n\t"
                     "rdtsc"
            : "=a"(lo), "=d"(hi)
            :
            : "memory");

        return (static_cast<uint64_t>(hi) << 32) | lo;
    }
}
