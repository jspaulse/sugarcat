#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include <stddef.h>

#ifdef ARCH_CPU_64
typedef uint64_t addr_t;
#define MAX_ADDRESS 0xFFFFFFFFFFFFFFFF
#else
typedef uint32_t addr_t;
#define MAX_ADDRESS 0xFFFFFFFF
#endif

#define PG_SZ	4096
#endif
