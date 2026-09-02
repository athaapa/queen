#include "boot_modules.hpp"
#include "limine.h"
#include "panic.hpp"

extern volatile struct limine_module_request module_request;

namespace {
    queen::boot_modules::Module program_module { };
}

void queen::boot_modules::capture() {
    auto* response = module_request.response;

    if (response == nullptr || response->module_count == 0) {
        queen::panic("program module missing");
    }

    struct limine_file* file = response->modules[0];

    program_module.address = file->address;
    program_module.size = file->size;
}

const queen::boot_modules::Module& queen::boot_modules::program() { return program_module; }
