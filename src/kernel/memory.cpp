#include "memory.hpp"
#include "limine.h"
#include "mathutil.hpp"
#include "panic.hpp"
#include "serial.hpp"

extern volatile struct limine_executable_address_request address_request;
extern volatile struct limine_memmap_request memmap_request;
extern "C" char kernel_start[];
extern "C" char kernel_end[];

constexpr uint64_t FRAME_SIZE_BYTES = 0x1000; // 4KB

uint64_t next_free_address = 0;
uint64_t memory_pool_base = 0;
uint64_t memory_pool_end = 0;

static void verify_pool(uint64_t base, uint64_t length, uint64_t type);

void queen::memory::init() {
    if (memmap_request.response == nullptr) {
        return;
    }

    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry** entries = memmap_request.response->entries;

    limine_memmap_entry* largest = nullptr;
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];
        if (entry->type == 0
            && (largest == nullptr || entry->length > largest->length)) { // Usable RAM
            largest = entry;
        }
    }

    if (largest == nullptr) {
        queen::panic("could not find any usable memory");
    }

    verify_pool(largest->base, largest->length, largest->type);
    queen::serial::write_line("allocator pool verified");

    next_free_address = largest->base;
    memory_pool_base = largest->base;
    memory_pool_end = largest->base + largest->length;

    dump_map();
}

void queen::memory::dump_map() {
    if (memmap_request.response == nullptr) {
        queen::serial::write("memory map unavailable\n");
        return;
    }

    queen::serial::write("memory map\n");
    uint64_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry** entries = memmap_request.response->entries;

    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry* entry = entries[i];
        queen::serial::write("  base: ");
        queen::serial::write_hex(entry->base);
        queen::serial::write(" length: ");
        queen::serial::write_hex(entry->length);
        queen::serial::write(" type: ");
        queen::serial::write_hex(entry->type);
        queen::serial::write("\n");
    }

    queen::serial::write("chosen pool base: ");
    queen::serial::write_hex(memory_pool_base);
    queen::serial::write(" end: ");
    queen::serial::write_hex(memory_pool_end);
    queen::serial::write("\n");
}

uint64_t queen::memory::get_base() { return memory_pool_base; }

uint64_t queen::memory::get_end() { return memory_pool_end; }

uint64_t queen::memory::allocate_frame() {
    uint64_t temp = next_free_address;
    next_free_address += FRAME_SIZE_BYTES;
    if (next_free_address >= memory_pool_end) {
        panic("You ran out of memory!");
    }

    return temp;
}

static void verify_pool(uint64_t base, uint64_t length, uint64_t type) {
    if (type != 0) {
        queen::panic("tried to allocate unusable memory");
    }
    if ((base & (FRAME_SIZE_BYTES - 1)) != 0) { // Is page-aligned?
        queen::panic("base is not page-aligned");
    }

    if (length == 0) {
        queen::panic("pool size must be >0");
    }

    uint64_t pool_end = base + length;
    if (pool_end <= base) {
        queen::panic("memory pool range overflow");
    }

    uint64_t start = reinterpret_cast<uint64_t>(kernel_start);
    uint64_t end = reinterpret_cast<uint64_t>(kernel_end);
    uint64_t aligned_start = queen::mathutil::align_down(start, FRAME_SIZE_BYTES);
    uint64_t aligned_end = queen::mathutil::align_up(end, FRAME_SIZE_BYTES);
    uint64_t aligned_size = aligned_end - aligned_start;

    uint64_t phys_base = address_request.response->physical_base;
    uint64_t phys_end = phys_base + aligned_size;

    if (queen::mathutil::max(base, phys_base) < queen::mathutil::min(pool_end, phys_end)) {
        queen::panic("pool overlaps with kernel physical range");
    }
}
