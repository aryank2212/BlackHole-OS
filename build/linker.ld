/* Linker script for BlackHole OS kernel (PE/Windows MinGW compatible) */

ENTRY(_start)

SECTIONS
{
    /* Kernel is loaded at 0x1000 by the bootloader */
    . = 0x1000;

    .text ALIGN(4) : {
        *(.text)
        *(.text.*)
    }

    .rdata ALIGN(4) : {
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4) : {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4) : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.comment)
        *(.drectve)
        *(.note.*)
        *(.reloc)
        *(.pdata)
        *(.xdata)
    }
}
