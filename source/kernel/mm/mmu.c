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
#include <mm/mmu.h>
#include <mm/mem.h>
#include <mm/mm.h>
#include <types.h>
#include <errno.h>
#include <stdbool.h>

static struct mm_resv_reg mmu_pg_tbs;

static int mmu_create_pgtb_entry(addr_t, addr_t, mmu_acc_flags_t, mmu_entry_type_t, bool);

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
// arch_mmu_get
// int mmu_create_entry(...)
// int mmu_map(...) ^
// int mmu_map_region(vaddr_region, phys_pgs[], pg_cnt)

// int mmu_map_new_entry(...)
// int mmu_map_new(...)^
// int mmu_map_new_region(...)

/* prototypes */

