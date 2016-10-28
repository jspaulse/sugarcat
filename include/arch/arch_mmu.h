#ifndef ARCH_MMU_H
#define ARCH_MMU_H
#include <types.h>
#include <mm/mmu.h>
#include <stdbool.h>
/**
 * set_user_pg_dir
 * 
 * Set User Page Directory
 * 
 * sets the user page dir (or table, depending on arch) to specified value
 * and invalidates the current tables
 * 
 * @pgd_addr		address to user page directory
 * 
 * @return		0 on success, non-zero otherwise
 **/
int arch_mmu_set_user_pg_dir(addr_t pgd_addr); 

/**
 * arch_set_kernel_pg_dir
 * 
 * Set Kernel Page Directory
 * 
 * sets the kernel page dir (or table, depending on arch) to specified value
 * and invalidates the current tables
 * 
 * @pgd_addr		address to kernel page directory
 * 
 * @return		0 on success, non-zero otherwise
 **/
int arch_mmu_set_kern_pg_dir(addr_t pgd_addr);

/**
 * arch_mmu_enable
 * 
 * enables the mmu
 * 
 * @return	0 on success, non-zero otherwise
 **/
int arch_mmu_enable(void);

/**
 * mmu_disable
 * 
 * disables the mmu
 * 
 * @return	zero on success, non-zero otherwise
 **/
int arch_mmu_disable(void);

/**
 * arch_mmu_invalidate_pg_dirs
 * 
 * invalidates page tables
 * 
 * this should be called whenever changes are made to tables
 *
 */
void arch_mmu_invalidate_pg_dirs(void);

/**
 * arch_mmu_map
 * 
 * maps a page to specified pg_dir
 * 
 * mme	mmu_entry structure
 * 
 * @return zero on success, non-zero on failure
 **/
int arch_mmu_map(struct mmu_entry *mme);

/**
 * arch_mmu_set_vm_map
 * 
 * sets the mmu map
 * 
 * @mvm	vmem map
 *
 * @return 0 on success, non-zero on failure
 **/
int arch_mmu_set_vm_map(mmu_vm_map mvm);

/**
 * arch_is_mmu_enabled
 * 
 * returns whether or not the mmu is enabled
 * 
 * @return true if mmu enabled, false otherwise
 **/
bool arch_is_mmu_enabled(void);


#endif
