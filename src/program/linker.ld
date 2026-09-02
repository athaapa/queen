ENTRY(queen_event_loop_entry)

SECTIONS
{
    . = 0x400000;

    .text : {
        *(.text .text.*)
    }
}
