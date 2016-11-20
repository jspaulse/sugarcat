#ifndef ARMV7_H
#define ARMV7_H
#include <types.h>

#define dmb() asm volatile("dmb" : : : "memory")
#define dsb() asm volatile("dsb" : : : "memory")
#define isb() asm volatile("isb" : : : "memory")

/* opcodes */
#define ARMV7_LDR_PC	0xE59FF000


#endif
