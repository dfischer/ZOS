ENTRY(start)
SECTIONS
{
	. = 0x100000;
	kernelStart = .;
	
	.setup :
	{
		*(.setup)
	}
	
	. += 0xC0000000;
	
	.multiboot_header : AT(ADDR(.multiboot_header) - 0xC0000000)
	{
		*(.multiboot_header)
	}
	
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}

	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
		*(.rodata*)
	}
	
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON*)
		*(.bss)
	}
	
	kernelEnd = . - 0xC0000000;
}
