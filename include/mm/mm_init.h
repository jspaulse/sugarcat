#ifndef MM_INIT_H
#define MM_INIT_H
#include <mm/mm.h>
#include <stddef.h>
#include <types.h>

/**
 * struct mm_resv_reg
 * 
 * defines a reserved region of memory.
 * 
 * @vbase	virtual address of base region
 * @phy		physical region
 **/
struct mm_resv_reg {
    addr_t		vbase;
    struct mm_reg	phy;
};

#endif
