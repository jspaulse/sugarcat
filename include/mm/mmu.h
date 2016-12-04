#ifndef MMU_H
#define MMU_H
#include <mm/mm.h>
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

typedef enum {
    PG_DIR		= 1,
    PG_TAB		= 2,
    PG_DIR_INVAL	= 3,
    PG_TAB_INVAL	= 4
} mmu_entry_type_t;

/**
 * mmu_entry
 * 
 * defines a mmu entry
 * @phy_addr	physical address of entry
 * @virt_addr	virtual address of entry 
 * 		or the physical address of page table
 * @type	entry type
 * @acc_flags	access type of entry
 **/
struct mmu_entry {
    addr_t 		phy_addr;
    addr_t		virt_addr;
    mmu_entry_type_t	type;
    mmu_acc_flags_t	acc_flags;
};

int mmu_interface_enable(struct mm_resv_reg *pg_tbs);

#endif


