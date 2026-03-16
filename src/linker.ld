ENTRY(_start)
SECTIONS
{
    . = 0x10000;
    _kernelStart = .;
    .text : { 
        *(.text) 
    }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) *(COMMON) }
    _kernelEnd = .;
}