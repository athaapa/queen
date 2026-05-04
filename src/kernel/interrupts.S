.section .text
.global isr0
.global isr14
.global isr32

// Divide-by-zero 
isr0:
    cli 
    call panic
    hlt
    iretq // Interrupt return -> tells the CPU that the interrupt has been dealt with 

// Tick
isr32:
    cli
    call tick
    mov $0x20, %al
    outb %al, $0x20
    iretq

// Page fault
isr14:
    cli 
    mov (%rsp), %rsi
    mov %cr2, %rdi
    call page_fault_handler
    iretq
