#ifndef ARMV7_H
#define ARMV7_H
#include <types.h>

#define dmb() asm volatile("dmb" : : : "memory")
#define dsb() asm volatile("dsb" : : : "memory")
#define isb() asm volatile("isb" : : : "memory")

/* opcodes */
#define ARMV7_LDR_PC	0xE59FF000

/* experimental */
inline unsigned int ldrex(unsigned int *ptr) {
    unsigned int ret = 0;

    asm volatile("ldrex %0, [%1]" : "=&r" (ret) : "r" (ptr));
    
    return ret;
}
    


#endif
