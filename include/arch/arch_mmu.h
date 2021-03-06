#ifndef ARCH_MMU_H
#define ARCH_MMU_H
#include <types.h>
#include <mm/mmu.h>
#include <stdbool.h>
#include <stddef.h>

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
 * must invalidate entries.
 * 
 * @page_dir	base address of new page directory
 * @return errno
 **/
extern int arch_mmu_set_user_pg_dir(addr_t page_dir);

/**
 * arch_mmu_create_entry
 * 
 * creates a mmu entry based on the parameters given in entry.
 * it is the responsibility of caller to use any necessary memory barriers
 * as well as invalidate tlbs (see arch_mmu_invalidate).
 * @entry	entry to add
 * @return errno
 **/
extern int arch_mmu_create_entry(struct mmu_entry *entry);

/**
 * arch_mmu_create_new_entry
 * 
 * creates a new mmu entry based on the parameters given in entry.
 * if entry->type == PG_DIR, pg_base should be the base address of the page directory.
 * if entry->type == PG_TAB, pg_base should be the base address of the continuous region
 * of page tables.
 * if entry-type == PG_TAB, this function must calculate where the page table of the entry
 * exists within the specified region.
 * this function is intended for new (unmapped) page tables & directories and must not be
 * used on the current mmu.
 * 
 * @pg_base	base address of either page directory or page tables
 * @entry	entry to add
 * @return errno
 **/
extern int arch_mmu_create_new_entry(addr_t pg_base, struct mmu_entry *entry);

/**
 * arch_mmu_invalidate
 * 
 * invalidates the current mmu entries.
 * this function should be called after making
 * alterations to the active mmu.
 **/
extern void arch_mmu_invalidate(void);

/**
 * arch_mmu_get_pgtb_reg_sz
 * 
 * returns the space required (in bytes) for creating
 * a continuous region of page tables required for mapping
 * user space.
 * @return space required (in bytes) for page tables required to map
 * user address space.
 **/
extern size_t arch_mmu_get_user_pgtb_reg_sz(void);

/**
 * arch_mmu_get_kern_pgtb_reg_sz
 * 
 * returns the space required (in bytes) for creating
 * a continuous region of page tables required for mapping
 * kernel space.
 * @return space required (in bytes) for page tables required to map
 * kernel address space.
 **/
extern size_t arch_mmu_get_kern_pgtb_reg_sz(void);

/**
 * arch_mmu_get_pgtb_alignment
 * 
 * returns the required alignment of a page table
 * this function must return the alignment in bytes,
 * i.e., for a 4KiB alignment, the function should return
 * 0x100 or 4096
 * 
 * @return required pgtb alignment
 **/
extern unsigned int arch_mmu_get_pgtb_alignment(void);

/**
 * arch_mmu_get_user_pgd_sz
 * 
 * returns the size (in bytes) required for
 * a user page directory.
 * 
 * @return size (in bytes) of user page directory.
 **/
extern size_t arch_mmu_get_user_pgd_sz(void);

/**
 * arch_mmu_user_pgd_requires_alignment
 * 
 * determines if the user page directory
 * requires a specific alignment.
 * 
 * @return true if alignment required
 **/
extern bool arch_mmu_user_pgd_requires_alignment(void);

/**
 * arch_mmu_get_user_pgd_alignment
 * 
 * returns the required alignment for a 
 * user page directory.
 * this function must return the alignment in bytes,
 * i.e., for a 16KiB alignment, the function should return
 * 0x4000 or 16384.
 * 
 * @return required user page directory alignment
 **/
extern unsigned int arch_mmu_get_user_pgd_alignment(void);

/**
 * arch_mmu_get_kern_vaddr
 * 
 * returns the beginning of the kernel virtual address space
 * @return base of kernel virtual address space.
 **/
extern addr_t arch_mmu_get_kern_vaddr(void);

#endif
