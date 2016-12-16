/* Copyright (C) 2016 Jacob Paulsen <jspaulse@ius.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <sync/barriers.h>
#include <arch/arch_mmu.h>
#include <util/bits.h>
#include <mm/mmu.h>
#include <mm/mem.h>
#include <mm/mm.h>
#include <types.h>
#include <errno.h>
#include <stdbool.h>

#define MMU_PG_SHIFT	12
#define MMU_PGD_SHIFT	20

#define DIV_MULT_MB 	20
#define DIV_MULT_PGTB	10
#define DIV_MULT_PG	12

/* keep track of kernel page tables */
static struct mm_resv_reg mmu_pg_tbs;

static int mmu_create_pgtb_entry(addr_t, addr_t, mmu_acc_flags_t, mmu_entry_type_t, bool);

/* require a lock on any access to the kernel regions */

/**
 * mmu_interface_enable
 * 
 * enables the mmu interface.
 * 
 * @pg_tbs	reserved region containing the kernel page tables
 * @return errno
 **/
int mmu_interface_enable(struct mm_resv_reg *pg_tbs) {
    int ret = ESUCC;
    
    if (pg_tbs != NULL) {
	memcpy(&mmu_pg_tbs, pg_tbs, sizeof(struct mm_resv_reg));
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * mmu_set_user_page_dir
 * 
 * sets the user page directory to new, specified page directory.
 * @page_dir	new page directory
 * @return errno
 **/
int mmu_set_user_page_dir(addr_t page_dir) {
    return arch_mmu_set_user_pg_dir(page_dir);
}

/**
 * mmu_invalidate_page
 * 
 * invalidates a page by removing the virtual->physical mapping.
 * @virt_addr	virtual address to invalidate
 * @return errno
 **/
int mmu_invalidate_page(addr_t virt_addr) {
    addr_t 		kvaddr 	= arch_mmu_get_kern_vaddr();
    mmu_acc_flags_t	acc;
    
    if (virt_addr >= kvaddr) {
	acc = KERNEL;
    } else {
	acc = USER;
    }
    
    return mmu_create_pgtb_entry(virt_addr, 0x0, acc, PG_TAB_INVAL, true);
}

/**
 * mmu_invalidate_region
 * invalidates a region of virtual memory by removing the virtual->physical mapping.
 * 
 * @virt_addr	base of virtual region to invalidate
 * @pg_cnt	number of pages to invalidate in region
 * @return errno
 **/
int mmu_invalidate_region(addr_t virt_addr, int pg_cnt) {
    int 		ret 	= ESUCC;
    int			i	= 0;
    addr_t		kvaddr	= arch_mmu_get_kern_vaddr();
    mmu_acc_flags_t	acc;
    
    if (pg_cnt > 0) {
	while ((ret == ESUCC) && (i < pg_cnt)) {
	    if (virt_addr >= kvaddr) {
		acc = KERNEL;
	    } else {
		acc = USER;
	    }
	    
	    ret = mmu_create_pgtb_entry(virt_addr, 0x0, acc, PG_TAB_INVAL, false);
	    virt_addr += PG_SZ;
	}
	
	/* if successful, invalidate */
	if (ret == ESUCC) {
	    /*
	     * ensure all data is written
	     * prior to invalidating the tlb
	     */
	    arch_dsb();
	    
	    /* invalidate */
	    arch_mmu_invalidate();
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * mmu_create_pgtb_entry
 * 
 * creates a page table entry based on the specified parameters.
 * 
 * @virt_addr	virtual address
 * @phy_addr	physical address
 * @acc_flags	access flags
 * @type	entry type
 * @invalidate	whether or not the tlb should be invalidated after write
 * @return errno
 **/
static int mmu_create_pgtb_entry(addr_t virt_addr, addr_t phy_addr, mmu_acc_flags_t acc_flags, mmu_entry_type_t type, bool invalidate) {
    int 		ret 	= ESUCC;
    struct mmu_entry	entry	= {
	.phy_addr	= phy_addr,
	.virt_addr	= virt_addr,
	.type		= type,
	.acc_flags	= acc_flags
    };
    
    ret = arch_mmu_create_entry(&entry);
    
    if ((ret == ESUCC) && invalidate) {
	arch_mmu_invalidate();
    }
    
    return ret;
}

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
extern bool arch_mmu_is_enabled(void);
extern addr_t virt_to_phy(addr_t virt_addr);
extern int arch_mmu_set_user_pg_dir(addr_t page_dir);
extern int arch_mmu_create_entry(struct mmu_entry *entry);
extern void arch_mmu_invalidate(void);
extern size_t arch_mmu_get_user_pgtb_reg_sz(void);
extern size_t arch_mmu_get_user_pgd_sz(void);
extern bool arch_mmu_user_pgd_requires_alignment(void);
extern unsigned int arch_mmu_get_user_pgd_alignment(void);
extern addr_t arch_mmu_get_kern_vaddr(void);

/* int mmu_map_page(addr_t virt_addr, addr_t phy_addr, mmu_acc_flags_t acc_flags) */
/* int mmu_map_region(addr_t virt_addr, addr_t *phy_pages, int pg_cnt, mmu_acc_flags_t acc_flags); */
/* int mmu_map_new_page(addr_t pgtb_base, addr_t virt_addr, addr_t phy_addr, mmc_acc_flags_t acc_flags) */
/* int mmu_map_new_region(addr_t pgtb_base, addr_t virt_addr, addr_t *phy_pages, int pg_cnt, mmu_acc_flags_t acc_flags) */
/* int mmu_create_new_user_pgd_pgtb(addr_t pg_dir, addr_t pg_tbs_base); */

