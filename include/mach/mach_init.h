#ifndef MACH_INIT_H
#define MACH_INIT_H
#include <types.h>

/* 
 * these are functions that are expected to be provided
 * if specified, some of them can be effectively empty.
 */

/**
 * mach_init_printf
 * 
 * used by the warn/panic utilities before a proper console is set up
 * this can output to whatever device is most useful
 * 
 * this can also be empty but warnings and errors will not be
 * available
 **/
void mach_init_printf(const char *fmt, ...);

#endif
