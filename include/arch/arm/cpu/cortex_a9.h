#ifndef CORTEX_A9_H
#define CORTEX_A9_H

/* private memory region offsets */
#define CORTEX_A9_SCU_OFFSET		0x0000
#define CORTEX_A9_GICC_OFFSET		0x0100
#define CORTEX_A9_GLOBAL_TIMER_OFFSET	0x0200
#define CORTEX_A9_PRIVATE_TIMER_OFFSET	0x0600
#define CORTEX_A9_GICD_OFFSET		0x1000

/**
 * cortex_a9_get_cpuid
 * 
 * returns the cpu id calling this function
 * 
 * @return cpu id
 **/
inline unsigned int cortex_a9_get_cpuid(void) {
    unsigned int ret = 0;
	
    asm volatile("mrc p15, 0, %0, c0, c0, 5" : "=r" (ret));
	
    return (ret & 0x3);
}

#endif
