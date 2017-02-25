OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_vexpress_boot)

/*
 * specify the physical start & virtual start
 * where the kernel will reside in memory
 */
kp_start	= 0x60000000;
kv_start 	= 0x80000000;

SECTIONS {
    . = kp_start; 		/* physical start */
    
    /* define region for kern pgd */
    . += 0x4000;		/* . = 0x60004000; */
    k_pgd = .;
    . += 0x4000;		/* . = 0x60008000; */
    
    /*
     * lm_init is the initialization section
     * for low memory; this region will be consumed
     * once initialization is complete
     */
    .lm_init : {
	lmi_start = .;
		
	/* text */
	build/vexpress_boot.o(.text)			/* vexpress_boot.s */
	build/vexpress_boot_init.o(.text .text.*)	/* vexpress_boot_init.c */
	build/vexpress_init.o(.text .text.*)		
	build/mach_early_kprintf.o(.text .text.*)
		
	/* rodata */
	build/vexpress_boot.o(.rodata .rodata.*)
	build/vexpress_boot_init.o(.rodata .rodata.*)
	build/vexpress_init.o(.rodata .rodata.*)
	build/mach_early_kprintf.o(.rodata .rodata.*)
		
	/* data */
	build/vexpress_boot.o(.data .data.*)
	build/vexpress_boot_init.o(.data .data.*)
	build/vexpress_init.o(.data .data.*)
	build/mach_early_kprintf.o(.data .data.*)
		
	/* bss */
	lmi_bss_start 	= .;
	build/vexpress_boot.o(.bss .bss.* COMMON)
	build/vexpress_boot_init.o(.bss .bss.* COMMON)
	build/vexpress_init.o(.bss .bss.* COMMON)
	build/mach_early_kprintf.o(.bss .bss.* COMMON)
	lmi_bss_end	= .;
	. = ALIGN(0x1000);
	
	/* k_stack */
	. += 0x1000;
	k_stack = .;

	/* align to page */
	. = ALIGN(0x1000);
	lmi_end = .;
    }
    
    /*
     * start of high memory (technically, still low physically)
     * this is where the majority of the kernel resides 
     */
    . = kv_start + (lmi_end - kp_start);

    /*
     * init region of main kernel;
     * this exists in high memory but will be consumed
     * once init is completed
     */
    .hm_init : AT(lmi_end) {
	hmi_start 	= .;
	
	build/kernel_init.o(.text .text.*)		/* kernel_init.c .text 	*/
	build/kernel_init.o(.rodata .rodata.*)		/* kernel_init.c .rodata */
	build/kernel_init.o(.data .data.*)		/* kernel_init.c .data 	*/
	
	hmi_bss_start 	= .;	
	build/vexpress_init.o(.bss .bss.* COMMON)	/* kernel_init.c .bss	*/
	hmi_bss_end	= .;
	
	/* align to page */
	. = ALIGN(0x1000);
	hmi_end		= .;
    }
    
    /*
     * this is where the rest of the kernel resides
     * the initialization routines will be consumed and
     * only this portion of the kernel will remain.
     */
    k_start = .;
    .text : AT((kp_start + (hmi_end - kv_start))) { 
	*(.text .text* .gnu.linkonce.t.*) 
    }
    
    .rodata : { *(.rodata*) } 
    .data : { *(.data*) } 
    .bss : { 
	k_bss_start 	= .;
	*(.bss .bss.* COMMON) 
	k_bss_end	= .;
    }
	
    . = ALIGN(0x1000);
    k_end = .;
}
