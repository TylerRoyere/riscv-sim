ENTRY(_start)

SECTIONS
{
    . = 0x80000000;
    .text.init :
    {
        *(.text.init)
    }
    .text :
    {
        *(.text)
    }
    . = ALIGN(0x1000);
    .data :
    {
        *(.data)
    }
    _end = .;
}
