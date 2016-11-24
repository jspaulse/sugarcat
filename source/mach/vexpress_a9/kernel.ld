OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_vexpress_boot)

kp_start	= 0x60000000;
kv_start 	= 0x80000000;
init_k_pgd	= 0x60300000;

SECTIONS {
	
	. = 0x60004000;
	k_pgd = .;
	. = 0x60008000;
	
	.start_sec : {
		ss_start = .;
		
		/* text */
		build/vexpress_boot.o(.text)
		build/vexpress_boot_init.o(.text .text.*)
		build/vexpress_init.o(.text .text.*)
		build/mach_init_printf.o(.text .text.*)
		
		/* rodata */
		build/vexpress_boot.o(.rodata .rodata.*)
		build/vexpress_boot_init.o(.rodata .rodata.*)
		build/vexpress_init.o(.rodata .rodata.*)
		build/mach_init_printf.o(.rodata .rodata.*)
		
		/* data */
		build/vexpress_boot.o(.data .data.*)
		build/vexpress_boot_init.o(.data .data.*)
		build/vexpress_init.o(.data .data.*)
		build/mach_init_printf.o(.data .data.*)
		
		/* bss */
		ss_bss_start = .;
		
		build/vexpress_boot.o(.bss .bss.* COMMON)
		build/vexpress_boot_init.o(.bss .bss.* COMMON)
		build/vexpress_init.o(.bss .bss.* COMMON)
		build/mach_init_printf.o(.bss .bss.* COMMON)

		ss_bss_end	= .;
		
		/* kernel stack */
		. = ALIGN(0x1000);
		. += 0x1000;
		kern_stack = .;

		. = ALIGN(0x1000);
		ss_end = .;
	}
	
	. = kv_start + (ss_end - kp_start);
	
	k_start = .;
	.text : AT(ss_end) { 
		*(.text .text* .gnu.linkonce.t.*) 
	}
	
	PROVIDE(etext = .);
	
	/* everything but boot */
    .rodata : { *(.rodata*) } 
    .data : { *(.data*) } 
    .bss : { 
		bss_start = .;
		*(.bss*) 
		bss_end	= .;
	}
	
	. = ALIGN(0x1000);
	k_end = .;
}
