.section .text
.global switch_stack

switch_stack:
    mov %rdi, %rsp
    mov %rsi, %rdi
    call paging_after_stack_switch

.hang:
    hlt
    jmp .hang
