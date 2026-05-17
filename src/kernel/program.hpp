#pragma once
#include <stdint.h>

namespace queen {
    namespace program {
        using Entry = uint64_t (*)(uint64_t id, uint64_t value);
    }
}
