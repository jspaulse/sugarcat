#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include <stddef.h>

#ifdef ARCH_CPU_64
typedef uint64_t addr_t;
#else
typedef uint32_t addr_t;
#endif

#endif
