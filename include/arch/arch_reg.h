#ifndef ARCH_REG_H
#define ARCH_REG_H
#include <types.h>

/**
 * arch_set_sp
 * 
 * Arch. Set Stack Pointer
 * 
 * Sets the stack point to specified address.
 * NOTE: this should be in context of the calling CPU
 * @address	address to set stack pointer to
 **/
extern void arch_set_sp(addr_t address);

#endif
