# queen

Queen is a small x86-64 OS/runtime for studying predictable tail latency.

It does not target POSIX, Linux binaries, or general-purpose Unix behavior. The
goal is to run Queen-native workloads and measure the cost of OS mechanisms:
page faults, allocation, program boundaries, syscalls, scheduling, and
user/kernel isolation.

## Status

Queen currently boots through Limine in QEMU, initializes GDT/IDT/PIC state,
builds its own page tables, switches to a Queen-owned stack, loads its own
PML4 into `cr3`, diagnoses page faults, seals boot frame allocation, and runs a
kernel-embedded event-loop benchmark.

QEMU is used for correctness. Cycle-level latency numbers need bare metal.

## Build

```sh
cmake -S . -B build
cmake --build build --target queen_iso
cmake --build build --target run_queen
```
