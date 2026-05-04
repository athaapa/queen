#pragma once
#include <stdint.h>

namespace queen {
    namespace paging {
        void init();
        void activate();
        void map_page(uint64_t virtual_address, uint64_t physical_address);
    }
}
