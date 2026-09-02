#include <stdint.h>

extern "C" uint64_t queen_event_loop_entry(uint64_t id, uint64_t value) { return id + 3 * value; }
