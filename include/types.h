#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PG_SZ	4096

#ifdef ARCH_CPU_64
typedef uint64_t addr_t;
#define MAX_ADDRESS 0xFFFFFFFFFFFFFFFF
#else
typedef uint32_t addr_t;
#define MAX_ADDRESS 0xFFFFFFFF
#endif

/**
 * is_kernel_cpu64
 * 
 * determines if the kernel is 32 or 64 bit.
 * 
 * @return true if 64 bit
 **/
inline bool is_kernel_cpu64(void) {
    bool ret = false;
    
    #ifdef ARCH_CPU_64
	ret = true;
    #endif
    
    return ret;
}
    

#endif
