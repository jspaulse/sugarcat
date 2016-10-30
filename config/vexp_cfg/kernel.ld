OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_armv7_boot)

/*
MEMORY {
	ram  : ORIGIN = 0x60000000, LENGTH = 1M
}
*/
kp_start	= 0x60000000;
kv_start 	= 0x80000000;

SECTIONS {
	
	. = 0x60008000;
	/* . = 0x8000; */
	
	.start_sec : {
		ss_start = .;
		
		/* text */
		build/armv7_boot.o(.text)
		build/armv7_init.o(.text .text.*)

		/* rodata */
		build/armv7_boot.o(.rodata .rodata.*)
		build/armv7_init.o(.rodata .rodata.*)
		
		/* data */
		build/armv7_boot.o(.data .data.*)
		build/armv7_init.o(.data .data.*)

		/* bss */
		ss_bss_start = .;
		build/armv7_boot.o(.bss .bss.* COMMON)
		build/armv7_init.o(.bss .bss.* COMMON)
		ss_bss_end	= .;
		
		. = ALIGN(0x1000);
		. += 0x1000;
		sys_stack = .;
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

/*

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(reset)

MEMORY {
    ram  : ORIGIN = 0x60008000, LENGTH = 32K
}

SECTIONS {
   .text : { *(.text*) } > ram
   .bss  : { *(.text*) } > ram
}
*/
