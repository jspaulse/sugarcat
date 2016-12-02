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
#include <arch/arm/armv7/armv7.h>
#include <arch/arm/armv7/armv7_mmu.h>
#include <arch/arm/armv7/armv7_syscntl.h>
#include <util/bits.h>
#include <types.h>
#include <errno.h>
#include <stdbool.h>

/* helper functions */
static void armv7_mmu_update(bool dsb);
static bool is_higher_half(addr_t virt_addr, int n);
static int get_pgd_entry(addr_t pgd_addr, addr_t virt_addr, struct armv7_mmu_pgd_entry *out);
static int create_pgtb_entry(addr_t pgtb_addr, struct armv7_mmu_pgtb_entry *entry);
static int create_pgd_entry(addr_t pgd_addr, struct armv7_mmu_pgd_entry *entry);

/**
 * armv7_mmu_virt_to_phy
 * 
 * returns the physical address associated with a virtual address.
 * if the virtual address isn't mapped to a physical address, returns 0.
 * 
 * @virt_addr	virtual address
 * @return physical address
 **/
addr_t armv7_mmu_virt_to_phy(addr_t virt_addr) {
    unsigned int	index	= 0;
    unsigned char	type	= 0;
    addr_t 		ret 	= 0x0;
    addr_t 		*pg_dir	= NULL;
    
    if (armv7_mmu_is_enabled()) {
	index = virt_addr >> PGD_IDX_SHIFT;
	
	if (is_higher_half(virt_addr, armv7_mmu_get_pg_div())) {
	    pg_dir = (addr_t *)armv7_mmu_get_kern_pgd();
	} else {
	    pg_dir = (addr_t *)armv7_mmu_get_user_pgd();
	}
	
	/* grab entry type */
	type = (pg_dir[index] & PGD_TYPE_MASK);
	
	/* determine entry type; get phy addr based on that */
	if (type == ARMV7_MMU_PGD_SECTION) {
	    ret = ((pg_dir[index] & PGD_SECT_MASK) | (virt_addr & ~PGD_SECT_MASK));
	} else if (type == ARMV7_MMU_PGD_TABLE) {
	    addr_t *pg_tb = (addr_t *)(pg_dir[index] & PGD_TABLE_MASK);
	    
	    /* table index */
	    index 	= (virt_addr >> PGTB_IDX_SHIFT) & PGTB_IDX_MASK;
	    type	= pg_tb[index] & PGTB_TYPE_MASK;
	    
	    if (type == ARMV7_MMU_PGTB_LARGE_PG) {
		ret = ((pg_tb[index] & PGTB_LG_PG_MASK) | (virt_addr & ~PGTB_LG_PG_MASK));
	    } else if (type == ARMV7_MMU_PGTB_SMALL_PG) {
		ret = ((pg_tb[index] & PGTB_SM_PG_MASK) | (virt_addr & ~PGTB_SM_PG_MASK));
	    }
	}
    } else {
	ret = virt_addr;
    }
    
    return ret;
}

/**
 * armv7_mmu_get_kern_pgd
 * 
 * returns the current kernel page directory
 * @return kern pgd
 **/
addr_t armv7_mmu_get_kern_pgd(void) {
    addr_t ret = 0x0;
    
    if (armv7_mmu_is_enabled()) {
	ret = armv7_get_ttbr1() & TTBR_MASK;
    }
    
    return ret;
}

/**
 * armv7_mmu_get_user_pgd
 * 
 * returns the current user page directory
 * @return user pgd
 **/
addr_t armv7_mmu_get_user_pgd(void) {
    addr_t 		ret 	= 0x0;
    int			pg_div	= 0;
    unsigned int	mask	= 0;
    
    if (armv7_mmu_is_enabled()) {
	pg_div 	= armv7_mmu_get_pg_div();
	mask 	= ~((1 << (TTBR_ALIGN - pg_div)) - 1);
	
	ret = armv7_get_ttbr0() & mask;
	
    }
    
    return ret;
}

/**
 * armv7_mmu_set_kern_pgd
 * 
 * sets the kernel page directory.
 * this requires pgd_addr be aligned (1 << 14)
 * 
 * @pgd_addr	page directory
 * @flags	associated flags
 * @return errno
 **/
int armv7_mmu_set_kern_pgd(addr_t pgd_addr, unsigned char flags) {
    unsigned int	align 	= 1 << TTBR_ALIGN;
    int			ret	= ESUCC;
    
    if (is_aligned_n(pgd_addr, align)) {
	armv7_set_ttbr1(pgd_addr | flags);
	
	if (armv7_mmu_is_enabled()) {
	    armv7_mmu_update(false);
	}
    } else {
	ret = EALIGN;
    }
    
    return ret;
}
    
/**
 * armv7_mmu_set_user_pgd
 * 
 * sets the user page directory.
 * this requires pgd_addr be aligned ((1 << (14 - pg_div)))
 * 
 * @pgd_addr	new page directory address
 * @flags	associated flags
 * @return errno
 **/
int armv7_mmu_set_user_pgd(addr_t pgd_addr, unsigned char flags) {
    unsigned int	align	= 1 << (TTBR_ALIGN - armv7_mmu_get_pg_div());
    int			ret	= ESUCC;
    
    if (is_aligned_n(pgd_addr, align)) {
	armv7_set_ttbr0(pgd_addr | flags);
	
	if (armv7_mmu_is_enabled()) {
	    armv7_mmu_update(false);
	}
    } else {
	ret = EALIGN;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_pgd
 * 
 * creates a page directory entry from specified structure.
 * this requires that the mmu is enabled prior to calls
 * to this function.
 * 
 * @pgd_ent	page directory entry
 * @return errno
 **/
int armv7_mmu_map_pgd(struct armv7_mmu_pgd_entry *pgd_ent) {
    addr_t	pgd_addr	= 0x0;
    int		pg_div		= 0;
    int 	ret 		= ESUCC;
    
    if (pgd_ent != NULL) {
	if (armv7_is_supported_pgd_type(pgd_ent->type)) {
	    if (armv7_mmu_is_enabled()) {
		pg_div = armv7_mmu_get_pg_div();
		
		if (is_higher_half(pgd_ent->virt_addr, pg_div)) {
		    pgd_addr = armv7_mmu_get_kern_pgd();
		} else {
		    pgd_addr = armv7_mmu_get_user_pgd();
		}
		
		ret = create_pgd_entry(pgd_addr, pgd_ent);
		
		if ((ret == ESUCC)) {
		    armv7_mmu_update(true);
		}
	    } else {
		ret = ENOTENB;
	    }
	} else {
	    ret = ENOTSUPP;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_new_pgd
 * 
 * creates a page directory entry from specified structure into a specified page directory.
 * this does not (and must not) effect the current operating state of the mmu.
 * 
 * @pgd_addr	address of specified page directory
 * @pgd_ent	page directory entry
 * @return errno
 **/
int armv7_mmu_map_new_pgd(addr_t pgd_addr, struct armv7_mmu_pgd_entry *pgd_ent) {
    int ret = ESUCC;
    
    if (pgd_ent != NULL) {
	if (armv7_is_supported_pgd_type(pgd_ent->type)) {
	    ret = create_pgd_entry(pgd_addr, pgd_ent);
	} else {
	    ret = ENOTSUPP;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_pgtb
 * 
 * creates a page table entry.
 * this requires that a page directory entry was created prior to
 * calls to this function.
 * this also requires that the mmu be enabled prior to calls
 * to this function.
 * 
 * @pgtb_ent	page table entry to create
 * @return errno
 **/
int armv7_mmu_map_pgtb(struct armv7_mmu_pgtb_entry *pgtb_ent) {
    addr_t 	pgd_addr	= 0x0;
    int		pg_div		= 0;
    int		ret		= ESUCC;
    
    if (pgtb_ent != NULL) {
	if (armv7_mmu_is_enabled()) {
	    pg_div = armv7_mmu_get_pg_div();
	    
	    if (armv7_is_supported_pgtb_type(pgtb_ent->type)) {
		struct armv7_mmu_pgd_entry pgd_ent;
		
		/* determine which page directory to use */
		if (is_higher_half(pgtb_ent->virt_addr, pg_div)) {
		    pgd_addr = armv7_mmu_get_kern_pgd();
		} else {
		    pgd_addr = armv7_mmu_get_user_pgd();
		}
		
		/* attempt to get entry */
		if ((ret = get_pgd_entry(pgd_addr, pgtb_ent->virt_addr, &pgd_ent)) == ESUCC) {
		    if (pgd_ent.type == ARMV7_MMU_PGD_TABLE) {
			ret = create_pgtb_entry(pgd_ent.phy_addr, pgtb_ent);
			
			if ((ret == ESUCC)) {
			    armv7_mmu_update(true);
			}
		    } else {
			ret = ENOTFND;
		    }
		}
	    } else {
		ret = ENOTSUPP;
	    }
	} else {
	    ret = ENOTENB;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_new_pgtb
 * 
 * creates a page table entry in specified page table.
 * this does not (and most not) effect the current operating state of the mmu.
 * 
 * @pgtb_addr	base address of page table
 * @pgtb_ent	page table entry to add
 * @return errno
 **/
int armv7_mmu_map_new_pgtb(addr_t pgtb_addr, struct armv7_mmu_pgtb_entry *pgtb_ent) {
    int ret = ESUCC;
    
    if (pgtb_ent != NULL) {
	if (armv7_is_supported_pgtb_type(pgtb_ent->type)) {
	    ret = create_pgtb_entry(pgtb_addr, pgtb_ent);
	} else {
	    ret = ENOTSUPP;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * is_higher_half
 * 
 * determines whether an address is higher half (i.e., uses ttbr1)
 * based on the address and the split N
 * 
 * @virt_addr	address to check
 * @pg_div_n	mmu split
 * @return true if higher
 **/
static bool is_higher_half(addr_t virt_addr, int pg_div_n) {
    addr_t	div		= 0;
    bool 	ret 		= false;
    
    if (pg_div_n > 0) {
	div = (1 << (32 - pg_div_n));
	
	if (virt_addr >= div) {
	    ret = true;
	}
    }
    
    return ret;
}

/**
 * armv7_mmu_update
 * 
 * causes the mmu to update to changes.
 * @dsb	determines if a data sync. barrier should be used
 **/
static void armv7_mmu_update(bool dsb) {
    /* if necessary, ensure all data is written
     * prior to invalidation */
    if (dsb) {
	dsb();
    }
    
    armv7_invalidate_unified_tlb();
}

/**
 * create_pgtb_entry
 * 
 * creates a page table entry from the specified page directory.
 * note: this does not create the mapped pgd entry
 * 
 * @pgd_addr	address of the page table
 * @entry	entry to add
 * @return errno
 **/
static int create_pgtb_entry(addr_t pgtb_addr, struct armv7_mmu_pgtb_entry *entry) {
    addr_t		*pg_tb	= (addr_t *)pgtb_addr;
    unsigned int 	index	= 0;
    unsigned int	wr_ent	= 0;
    int			ret	= ESUCC;
    
    if (entry != NULL) {
	index 	= (entry->virt_addr >> PGTB_IDX_SHIFT) & PGTB_IDX_MASK;
	wr_ent	= entry->phy_addr;
	
	if (entry->type == ARMV7_MMU_PGTB_LARGE_PG) {
	    wr_ent &= PGTB_LG_PG_MASK;
	} else if (entry->type == ARMV7_MMU_PGTB_SMALL_PG) {
	    wr_ent &= PGTB_SM_PG_MASK;
	}
	
	pg_tb[index] = wr_ent | (entry->acc_perm << PGTB_AP_SHIFT) | entry->flags | entry->type;
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * create_pgd_entry
 * 
 * creates a page directory entry in specified page directory
 * 
 * @pgd_addr	address to page directory
 * @entry	entry to add
 * @return errno
 **/
static int create_pgd_entry(addr_t pgd_addr, struct armv7_mmu_pgd_entry *entry) {
    addr_t 		*pg_dir = (addr_t *)pgd_addr;
    unsigned int	index	= 0;
    unsigned int	wr_ent	= 0;
    int			ret	= ESUCC;
    
    if (entry != NULL) {
	index = (entry->virt_addr >> PGD_IDX_SHIFT);
	
	/* mask & assign bits */
	switch (entry->type) {
	    case ARMV7_MMU_PGD_SECTION:
		wr_ent	= (entry->phy_addr & PGD_SECT_MASK);
		wr_ent	|= (entry->acc_perm << PGD_SECT_AP_SHIFT);
		break;
	    case ARMV7_MMU_PGD_TABLE:
		wr_ent	= (entry->phy_addr & PGD_TABLE_MASK);
		break;
	    case ARMV7_MMU_PGD_SUPER_SECTION:
	    case ARMV7_MMU_PGD_INVALID:
		/* NOT SUPPORTED */
		break;
	}
	
	pg_dir[index] = wr_ent | (entry->domain << PGD_DOMAIN_SHIFT) | entry->flags | entry->type;
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * get_pgd_entry
 * 
 * returns the page directory entry for specified virtual address
 * 
 * @pgd_addr	page directory base address
 * @virt_addr	virtual address
 * @out		structure to output entry
 * @return errno
 **/
static int get_pgd_entry(addr_t pgd_addr, addr_t virt_addr, struct armv7_mmu_pgd_entry *out) {
    addr_t 		*pg_dir	= (addr_t *)pgd_addr;
    unsigned int	index	= 0;
    int			ret	= 0;
    unsigned char	type	= 0;
    unsigned int	flg_msk	= 0;
    
    if (out != NULL) {
	index 	= virt_addr >> PGD_IDX_SHIFT;
	type	= pg_dir[index] & PGD_TYPE_MASK;
	
	if (type == ARMV7_MMU_PGD_SECTION) {
	    flg_msk		= ~(PGD_SECT_MASK | PGD_DOMAIN_MASK | PGD_AP_MASK | PGD_TYPE_MASK);
	    
	    out->phy_addr 	= (pg_dir[index] & PGD_SECT_MASK);
	    out->domain		= (pg_dir[index] & PGD_DOMAIN_MASK) >> PGD_DOMAIN_SHIFT;
	    out->acc_perm	= (pg_dir[index] & PGD_AP_MASK) >> PGD_SECT_AP_SHIFT;
	    out->type		= (pg_dir[index] & PGD_TYPE_MASK);
	    out->flags		= (pg_dir[index] & flg_msk);
	    out->virt_addr	= virt_addr;
	} else if (type == ARMV7_MMU_PGD_TABLE) {
	    flg_msk		= ~(PGD_TABLE_MASK | PGD_DOMAIN_MASK | PGD_TYPE_MASK);
	    
	    out->phy_addr	= (pg_dir[index] & PGD_TABLE_MASK);
	    out->domain		= (pg_dir[index] & PGD_DOMAIN_MASK) >> PGD_DOMAIN_SHIFT;
	    out->acc_perm	= 0;
	    out->type		= (pg_dir[index] & PGD_TYPE_MASK);
	    out->flags		= (pg_dir[index] & flg_msk);
	    out->virt_addr	= virt_addr;
	} else { /* NOT SUPPORTED */
	    ret = ENOTFND;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}
