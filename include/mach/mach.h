/* mach.h 
 * Contains external definitions of functions that should be provided;
 * some reside within config definitions and are only required to be provided
 * if that configuration flag is used.
 */
#ifndef MACH_H
#define MACH_H
#include <types.h>


#ifdef CONFIG_EARLY_KPRINTF

/**
 * mach_early_kprintf
 * 
 * this function provides printf capabilities before a proper console
 * and device is setup.
 * 
 * @fmt	formatted string
 **/
extern void mach_early_kprintf(const char *fmt, ...);
#endif



#endif
