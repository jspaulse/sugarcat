OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(reset)
kv_start 	= 0x80000000;

SECTIONS {
	. = 0x8000;
	
	.start_sec : {
		ss_start = .;
		/* text */
		build/armv6_boot.o(.text)
		build/armv6_init.o(.text .text.*)

		/* rodata */
		build/armv6_boot.o(.rodata .rodata.*)
		build/armv6_init.o(.rodata .rodata.*)
		
		/* data */
		build/armv6_boot.o(.data .data.*)
		build/armv6_init.o(.data .data.*)

		/* bss */
		ss_bss_start = .;
		build/armv6_boot.o(.bss .bss.* COMMON)
		build/armv6_init.o(.bss .bss.* COMMON)
		ss_bss_end	= .;
		
		. = ALIGN(0x1000);
		. += 0x1000;
		svc_stack = .;
		ss_end = .;
	}
	. = kv_start + ss_end;
	
	k_start = .;
	.text : AT(ss_end) { 
		*(.text .text* .gnu.linkonce.t.*) 
	}
	
	PROVIDE(etext = .);
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
SECTIONS {
	k_start = .;
    .text : { *(.text*) } > ram
    .rodata : { *(.rodata*) } > ram
    .data : { *(.data*) } > ram
    .bss : { 
		bss_start = .;
		*(.bss*) 
		bss_end	= .;
	} > ram
    . = ALIGN(0x2000);
    PROVIDE (mmu_kpgdir = .);
    . += 0x2000;
    PROVIDE(mmu_upgdir = .);
    . += 0x2000;
    k_end = .;
}
*/
