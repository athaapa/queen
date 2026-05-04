#pragma once
#include <stdint.h>

namespace queen {
    namespace paging {
        void init();
        void map_page(uint64_t virtual_address, uint64_t physical_address);
        uint64_t get_hhdm_offset();
        uint64_t get_pml4_phys();
        uint64_t get_kernel_stack_top();
    }
}
