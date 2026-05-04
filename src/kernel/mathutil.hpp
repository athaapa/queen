#pragma once
#include <stdint.h>

namespace queen {
    namespace mathutil {
        // Aligns x to the nearest multiple of y that is < x
        uint64_t align_down(uint64_t x, uint64_t y);

        // Aligns x to the nearest multiple of y that is > x
        uint64_t align_up(uint64_t x, uint64_t y);

        // Returns the max of two values x, y
        uint64_t max(uint64_t x, uint64_t y);

        // Returns the min of two values x, y
        uint64_t min(uint64_t x, uint64_t y);
    }
}
