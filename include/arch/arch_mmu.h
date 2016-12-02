#ifndef ARCH_MMU_H
#define ARCH_MMU_H
#include <stdbool.h>
#include <types.h>
#include <mm/mmu.h>

/**
 * arch_mmu_is_enabled
 * 
 * determines if the mmu is currently enabled
 * @return true if enabled
 **/
extern bool arch_mmu_is_enabled(void);

/**
 * virt_to_phy
 * 
 * translates a virtual address into it's corresponding physical address.
 * this function should return 0x0 if not found or the virt_addr if the mmu
 * is not enabled.
 * 
 * @virt_addr	virtual address to translate
 * @return corresponding physical address
 **/
extern addr_t virt_to_phy(addr_t virt_addr);

/**
 * arch_mmu_set_user_pg_dir
 * 
 * sets the user page directory.
 * if the mmu is currently enabled, this function
 * must invalidate entries
 * 
 * @page_dir	base address of new page directory
 * @return errno
 **/
extern int arch_mmu_set_user_pg_dir(addr_t page_dir);

/**
 * arch_mmu_create_pgtb_entry
 * 
 * creates a mmu pgtb entry based on the parameters given in entry.
 * if the mmu is currently enabled, this function must
 * invalidate entries.
 * @entry	entry to add
 * @return errno
 **/
extern int arch_mmu_create_pgtb_entry(struct mmu_entry *entry);

/**
 * arch_mmu_create_pgd_entry
 * 
 * creates a mmu pgd entry based on the parameters given in entry.
 * if the mmu is currently enabled, this function must
 * invalidate entries.
 * @entry	entry to add
 * @return errno
 **/
extern int arch_mmu_create_pgd_entry(struct mmu_entry *entry);



#endif
