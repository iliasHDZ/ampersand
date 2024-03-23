ENTRY(_start)

SECTIONS
{
    . = 1M;

    .boot :
    {
        KEEP(*(.boot))
    }

	. = 0xC0000000;
	.text ALIGN (4K) : AT ( ADDR(.text) - 0xC0000000 + ADDR (.boot) + SIZEOF (.boot) )
	{
        kernel_begin_phys = ADDR(.text) - 0xC0000000 + ADDR (.boot) + SIZEOF (.boot);
        kernel_begin_addr = .;
		*(.text)
	}
	.rodata ALIGN (4K) : AT ( ADDR(.rodata) - 0xC0000000 + ADDR (.boot) + SIZEOF (.boot) )
	{
		*(.rodata)
	}
	.data ALIGN (4K) : AT ( ADDR(.data) - 0xC0000000 + ADDR (.boot) + SIZEOF (.boot) )
	{
		*(.data)
	}
	.bss ALIGN (4K) : AT ( ADDR(.bss) - 0xC0000000 + ADDR (.boot) + SIZEOF (.boot) )
	{
		*(COMMON)
		*(.bss)
        kernel_end_phys = ADDR(.bss) - 0xC0000000 + SIZEOF(.bss) + ADDR (.boot) + SIZEOF (.boot);
        kernel_end_addr = .;
	}
}