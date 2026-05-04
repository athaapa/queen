.section .text
.global switch_stack

switch_stack:
    mov %rdi, %rsp
    mov %rsi, %rdi
    call kernel_after_paging_switch

.hang:
    hlt
    jmp .hang
