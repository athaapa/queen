#include "mathutil.hpp"

uint64_t queen::mathutil::align_down(uint64_t x, uint64_t y) { return x & ~(y - 1); }

uint64_t queen::mathutil::align_up(uint64_t x, uint64_t y) { return (x + y - 1) & ~(y - 1); }

uint64_t queen::mathutil::max(uint64_t x, uint64_t y) {
    if (x < y) {
        return y;
    }
    return x;
}

uint64_t queen::mathutil::min(uint64_t x, uint64_t y) {
    if (x < y) {
        return x;
    }
    return y;
}
