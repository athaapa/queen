#include "paging.hpp"
#include "kernel_main.hpp"
#include "limine.h"
#include "mathutil.hpp"
#include "memory.hpp"
#include "serial.hpp"
#include <stdint.h>

extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_executable_address_request address_request;
extern "C" char kernel_start[];
extern "C" char kernel_end[];
extern "C" void switch_stack(uint64_t stack_top, uint64_t pml4_phys);
extern "C" void load_cr3(uint64_t pml4_phys);
extern "C" void kernel_after_paging_switch(uint64_t pml4_phys);
static uint64_t current_cr3();
static uint64_t kernel_stack_top();
static void run_post_switch_self_test(uint64_t active_pml4_phys);
constexpr uint64_t STACK_SIZE = 16384;
constexpr uint64_t PAGE_SIZE = 4096;

constexpr uint64_t INDEX_MASK = 0x1FF;

constexpr uint64_t NO_FLAGS = ~0xFFFULL;
constexpr uint64_t PRESENT_FLAG = 0x01;
constexpr uint64_t RW_FLAG = 0x02;

uint64_t pml4_phys = 0;
uint64_t hhdm_offset = 0;
uint64_t* pml4_virt = nullptr;

alignas(16) static uint8_t kernel_stack[STACK_SIZE];

// Format and print an address range
static void print_range(const char* name, uint64_t virt_start, uint64_t virt_end,
    uint64_t phys_start, uint64_t phys_end);

extern "C" void kernel_after_paging_switch(uint64_t pml4_phys) {
    load_cr3(pml4_phys);
    run_post_switch_self_test(pml4_phys);
    queen::kernel_runtime_main();
}

void queen::paging::init() {
    pml4_phys = queen::memory::allocate_frame();

    hhdm_offset = hhdm_request.response->offset;
    pml4_virt = (uint64_t*)(pml4_phys + hhdm_offset);

    for (int i = 0; i < 512; i++) {
        pml4_virt[i] = 0;
    }

    uint64_t start = reinterpret_cast<uint64_t>(kernel_start);
    uint64_t end = reinterpret_cast<uint64_t>(kernel_end);

    uint64_t phys_base = address_request.response->physical_base;
    uint64_t virt_base = address_request.response->virtual_base;

    uint64_t mem_base = queen::memory::pool_base();
    uint64_t mem_end = queen::memory::pool_end();

    uint64_t kernel_virt_start = queen::mathutil::align_down(start, PAGE_SIZE);
    uint64_t kernel_virt_end = queen::mathutil::align_up(end, PAGE_SIZE);
    uint64_t kernel_phys_start = phys_base + (kernel_virt_start - virt_base);
    uint64_t kernel_phys_end = phys_base + (kernel_virt_end - virt_base);

    // Map the kernel
    for (uint64_t virt_addr = kernel_virt_start; virt_addr < kernel_virt_end;
        virt_addr += PAGE_SIZE) {
        uint64_t phys_addr = phys_base + (virt_addr - virt_base);
        map_page(virt_addr, phys_addr);
    }

    uint64_t hhdm_virt_start = mem_base + hhdm_offset;
    uint64_t hhdm_virt_end = mem_end + hhdm_offset;

    // HHDM mapping
    for (uint64_t phys_addr = mem_base; phys_addr < mem_end; phys_addr += PAGE_SIZE) {
        uint64_t virt_addr = phys_addr + hhdm_offset;
        map_page(virt_addr, phys_addr);
    }

    queen::serial::write("paging init\n");
    queen::serial::write("kernel_start: ");
    queen::serial::write_hex(start);
    queen::serial::write("\n");
    queen::serial::write("kernel_end: ");
    queen::serial::write_hex(end);
    queen::serial::write("\n");
    queen::serial::write("kernel_size: ");
    queen::serial::write_hex(end - start);
    queen::serial::write("\n");
    queen::serial::write("executable_phys_base: ");
    queen::serial::write_hex(phys_base);
    queen::serial::write("\n");
    queen::serial::write("executable_virt_base: ");
    queen::serial::write_hex(virt_base);
    queen::serial::write("\n");
    queen::serial::write("hhdm_offset: ");
    queen::serial::write_hex(hhdm_offset);
    queen::serial::write("\n");
    queen::serial::write("pml4_phys: ");
    queen::serial::write_hex(pml4_phys);
    queen::serial::write("\n");
    queen::serial::write("pml4_virt: ");
    queen::serial::write_hex(reinterpret_cast<uint64_t>(pml4_virt));
    queen::serial::write("\n");
    print_range(
        "kernel_map", kernel_virt_start, kernel_virt_end, kernel_phys_start, kernel_phys_end);
    print_range("hhdm_pool_map", hhdm_virt_start, hhdm_virt_end, mem_base, mem_end);
}

void queen::paging::activate() { switch_stack(kernel_stack_top(), pml4_phys); }

void queen::paging::map_page(uint64_t virtual_address, uint64_t physical_address) {
    uint16_t pt_index = (virtual_address >> 12) & INDEX_MASK;
    uint16_t pd_index = (virtual_address >> 21) & INDEX_MASK;
    uint16_t pdpt_index = (virtual_address >> 30) & INDEX_MASK;
    uint16_t pml4_index = (virtual_address >> 39) & INDEX_MASK;

    if (pml4_virt[pml4_index] == 0) {
        uint64_t pdpt_phys = queen::memory::allocate_frame();
        uint64_t* pdpt_virt = (uint64_t*)(pdpt_phys + hhdm_offset);

        for (int i = 0; i < 512; i++)
            pdpt_virt[i] = 0;

        pml4_virt[pml4_index] = pdpt_phys | (PRESENT_FLAG | RW_FLAG);
    }

    uint64_t pdpt_phys = pml4_virt[pml4_index] & NO_FLAGS;
    uint64_t* pdpt_virt = (uint64_t*)(pdpt_phys + hhdm_offset);

    if (pdpt_virt[pdpt_index] == 0) {
        uint64_t pd_phys = queen::memory::allocate_frame();
        uint64_t* pd_virt = (uint64_t*)(pd_phys + hhdm_offset);

        for (int i = 0; i < 512; i++)
            pd_virt[i] = 0;

        pdpt_virt[pdpt_index] = pd_phys | (PRESENT_FLAG | RW_FLAG);
    }

    uint64_t pd_phys = pdpt_virt[pdpt_index] & NO_FLAGS;
    uint64_t* pd_virt = (uint64_t*)(pd_phys + hhdm_offset);

    if (pd_virt[pd_index] == 0) {
        uint64_t pt_phys = queen::memory::allocate_frame();
        uint64_t* pt_virt = (uint64_t*)(pt_phys + hhdm_offset);

        for (int i = 0; i < 512; i++)
            pt_virt[i] = 0;

        pd_virt[pd_index] = pt_phys | (PRESENT_FLAG | RW_FLAG);
    }

    uint64_t pt_phys = pd_virt[pd_index] & NO_FLAGS;
    uint64_t* pt_virt = (uint64_t*)(pt_phys + hhdm_offset);

    pt_virt[pt_index] = physical_address | (PRESENT_FLAG | RW_FLAG);
}

static void print_range(const char* name, uint64_t virt_start, uint64_t virt_end,
    uint64_t phys_start, uint64_t phys_end) {
    queen::serial::write(name);
    queen::serial::write(" virt [");
    queen::serial::write_hex(virt_start);
    queen::serial::write(", ");
    queen::serial::write_hex(virt_end);
    queen::serial::write(") phys [");
    queen::serial::write_hex(phys_start);
    queen::serial::write(", ");
    queen::serial::write_hex(phys_end);
    queen::serial::write(")\n");
}

static uint64_t kernel_stack_top() { return reinterpret_cast<uint64_t>(kernel_stack + STACK_SIZE); }

static uint64_t current_cr3() {
    uint64_t value = 0;
    asm("movq %%cr3, %0" : "=r"(value));
    return value;
}

static void run_post_switch_self_test(uint64_t active_pml4_phys) {
    if (current_cr3() == active_pml4_phys) {
        queen::serial::write_line("cr3 equals pml4_phys");
    } else {
        queen::serial::write_line("cr3 does NOT equal pml4_phys");
    }

    uint64_t phys = queen::memory::allocate_frame();
    uint64_t* virt = (uint64_t*)(phys + hhdm_offset);
    *virt = 0x1234;

    uint64_t val = *virt;
    queen::serial::write_hex(val);
    queen::serial::write("\n");
}
