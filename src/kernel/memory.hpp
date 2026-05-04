#pragma once
#include <stdint.h>

namespace queen {
    namespace memory {
        void init();
        void dump_map();
        uint64_t allocate_frame();
        void seal_boot_allocations();

        uint64_t pool_base();
        uint64_t pool_end();
    }
}
