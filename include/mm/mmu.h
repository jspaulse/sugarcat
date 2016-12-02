#ifndef MMU_H
#define MMU_H
#include <types.h>

/**
 * mmu_acc_flags_t
 * 
 * defines access flags for entries
 **/
typedef enum {
    USER	= 1,		/* implicit kernel rw, user rw */
    KERN_USER	= 2,		/* implicit kernel rw, user ro */
    KERNEL	= 3,		/* explicit kernel rw, user no */
    KERNEL_RO	= 4,		/* explicit kernel ro, user no */
    DEVICE	= 5		/* implicit kernel rw, user no */
} mmu_acc_flags_t;

/**
 * mmu_entry
 * 
 * defines a mmu entry
 * @phy_addr	physical address of entry
 * @virt_addr	virtual address of entry
 * @acc_flags	access type of entry
 **/
struct mmu_entry {
    addr_t 		phy_addr;
    addr_t		virt_addr;
    mmu_acc_flags_t	acc_flags;
};

#endif


