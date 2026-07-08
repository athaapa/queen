ENTRY(kernel_main)

SECTIONS
{
    
    . = 0xffffffff80000000;
    kernel_start = .;

    .limine_requests : ALIGN(4K)
    {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    }

    .text : ALIGN(4K)
    {
        *(.text .text.*)
    }

    .queen_program : ALIGN(4K)
    {
        queen_program_start = .;
        KEEP(*(.queen_program))
    }
    queen_program_end = ADDR(.queen_program) + ((SIZEOF(.queen_program) + 0xfff) & ~0xfff);
    . = queen_program_end;

    .rodata : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(4K)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    kernel_end = .;
}
