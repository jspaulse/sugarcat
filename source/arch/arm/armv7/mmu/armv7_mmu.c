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
#include <mach/mach_init.h>	/* tmp */
#include <arch/arm/armv7/armv7.h>
#include <arch/arm/armv7/armv7_mmu.h>
#include <arch/arm/armv7/armv7_syscntl.h>
#include <mach/mach_init.h>
#include <types.h>
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>

/* keep track of various things */
static bool init	= false;
static addr_t user_pgd 	= 0, kern_pgd = 0;
static int pg_div 	= 0;	/* mmu split */

/* various helpful things */
static unsigned int ttbr0_mask = 0;

/* helper functions */
static void armv7_mmu_update(void);
static unsigned int make_ttbr0_mask(int n);
static bool is_higher_half(addr_t virt_addr, int n);
static int get_pgd_entry(addr_t pgd_addr, addr_t virt_addr, struct armv7_mmu_pgd_entry *out);
static int create_pgtb_entry(addr_t pgtb_addr, struct armv7_mmu_pgtb_entry *entry);
static int create_pgd_entry(addr_t pgd_addr, struct armv7_mmu_pgd_entry *entry);

/**
 * armv7_mmu_is_init
 * 
 * determines if the mmu has been initialized
 * 
 * @return true if init'd.
 **/
bool armv7_mmu_is_init(void) {
    return init;
}

/**
 * armv7_mmu_get_pg_div
 * 
 * returns the current page divide of the mmu
 * this requires the mmu have been previously init'd.
 * @return mmu pg_div
 **/
int armv7_mmu_get_pg_div(void) {
    return pg_div;
}

addr_t armv7_get_kern_pgd(void) {
    return kern_pgd;
}

/**
 * armv7_mmu_set_kern_pgd
 * 
 * sets the kernel page directory
 * 
 * @pgd_addr	new page directory address
 * @flags	associated flags
 **/
void armv7_mmu_set_kern_pgd(addr_t pgd_addr, unsigned char flags) {
    kern_pgd = pgd_addr;
    armv7_set_ttbr1(pgd_addr | flags);
    
    /* update if we're enabled */
    if (armv7_is_mmu_enabled()) {
	armv7_mmu_update();
    }
}

/**
 * armv7_mmu_set_user_pgd
 * 
 * sets the user page directory
 * 
 * @pgd_addr	new page directory address
 * @flags	associated flags
 **/
void armv7_mmu_set_user_pgd(addr_t pgd_addr, unsigned char flags) {
    if (pg_div > 0) {
	ttbr0_mask = make_ttbr0_mask(pg_div);
    } else {
	ttbr0_mask	= TTBR_MASK;
    }
    
    user_pgd = pgd_addr & ttbr0_mask;
    armv7_set_ttbr0(user_pgd | flags);
    
    if (armv7_is_mmu_enabled()) {
	armv7_mmu_update();
    }
}

/**
 * armv7_mmu_init_post_enable
 * 
 * called whenever the mmu is enabled prior to init
 * but mmu needs to collect information
 * 
 * @return errno
 **/
int armv7_mmu_init_post_enable(void) {
    int ret = ERR_SUCC;
    
    /* 
     * assumption is we're already enabled,
     * collect various mmu settings
     */
    if (armv7_is_mmu_enabled()) {
	pg_div = armv7_get_ttbcr() & PG_DIV_MASK;
	
	/* we're using more than ttbr0 */
	if (pg_div > 0) {
	    ttbr0_mask = make_ttbr0_mask(pg_div);
	    
	    /* grab the ttbr0/1 address */
	    user_pgd = armv7_get_ttbr0() & ttbr0_mask;
	    kern_pgd = armv7_get_ttbr1() & TTBR_MASK;
	} else {
	    ttbr0_mask	= TTBR_MASK;
	    user_pgd 	= armv7_get_ttbr0() & TTBR_MASK;
	}
	
	/* keep track of whether or not we are init. */
	init = true;
    } else {
	ret = ERR_INVAL;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_pgd
 * 
 * creates a page directory entry from specified structure.
 * this requires that the mmu is initialized prior to calls
 * to this function.
 * 
 * @pgd_ent	page directory entry
 * @return errno
 **/
int armv7_mmu_map_pgd(struct armv7_mmu_pgd_entry *pgd_ent) {
    addr_t	pgdir	= 0;
    int 	ret 	= ERR_SUCC;
    
    if (init) {
	if (pgd_ent != NULL) {
	    if (armv7_is_supported_pgd_type(pgd_ent->type)) {
		if (is_higher_half(pgd_ent->virt_addr, pg_div)) {
		    pgdir = kern_pgd;
		} else {
		    pgdir = user_pgd;
		}
	    
		ret = create_pgd_entry(pgdir, pgd_ent);
	    
		if (armv7_is_mmu_enabled() & (ret == ERR_SUCC)) {
		    armv7_mmu_update();
		}
	    } else {
		ret = ERR_NOTSUPP;
	    }
	} else {
	    ret = ERR_INVAL;
	}
    } else {
	ret = ERR_NOTINIT;
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
    int ret = ERR_SUCC;
    
    if (pgd_ent != NULL) {
	if (armv7_is_supported_pgd_type(pgd_ent->type)) {
	    ret = create_pgd_entry(pgd_addr, pgd_ent);
	} else {
	    ret = ERR_NOTSUPP;
	}
    } else {
	ret = ERR_INVAL;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_pgtb
 * 
 * creates a page table entry.
 * this requires that a page directory entry was created prior to
 * calls to this function.
 * this also requires that the mmu be initialized prior to calls
 * to this function.
 * 
 * @pgtb_ent	page table entry to create
 * @return errno
 **/
int armv7_mmu_map_pgtb(struct armv7_mmu_pgtb_entry *pgtb_ent) {
    addr_t 	pgdir	= 0;
    int		ret	= ERR_SUCC;
    
    if (init) {
	if (pgtb_ent != NULL) {
	    if (armv7_is_supported_pgtb_type(pgtb_ent->type)) {
		struct armv7_mmu_pgd_entry pgd_ent;
		
		/* determine pg dir to use */
		if (is_higher_half(pgtb_ent->virt_addr, pg_div)) {
		    pgdir = kern_pgd;
		} else {
		    pgdir = user_pgd;
		}
		
		/* attempt to get entry */
		if ((ret = get_pgd_entry(pgdir, pgtb_ent->virt_addr, &pgd_ent)) == ERR_SUCC) {
		    if (pgd_ent.type == ARMV7_MMU_PGD_TABLE) {
			ret = create_pgtb_entry(pgd_ent.phy_addr, pgtb_ent);
			
			if ((ret == ERR_SUCC) && armv7_is_mmu_enabled()) {
			    armv7_mmu_update();
			}
		    } else {
			ret = ERR_NOTFND;
		    }
		}
	    } else {
		ret = ERR_NOTSUPP;
	    }
	} else {
	    ret = ERR_INVAL;
	}
    } else {
	ret = ERR_NOTINIT;
    }
    
    return ret;
}


/**
 * armv7_mmu_map_new_pgd_pgtb
 * 
 * creates both a new page directory and page table entry based on specified structures.
 * this (logically) requires that pgd_ent->phy_addr is the page table
 * and pgd_ent->virt_addr == pgtb_ent->virt_addr.
 * this does not (and must not) effect the current operating state of the mmu.
 * 
 * @pgdir	page directory base address
 * @pgd_ent	page directory entry
 * @pgtb_ent	page table entry
 * @return errno
 **/
int armv7_mmu_map_new_pgd_pgtb(addr_t pgdir, struct armv7_mmu_pgd_entry *pgd_ent, struct armv7_mmu_pgtb_entry *pgtb_ent) {
    int ret = ERR_SUCC;
    
    if (pgd_ent != NULL && pgtb_ent != NULL) {
	if (pgd_ent->type == ARMV7_MMU_PGD_TABLE) {
	    if (armv7_is_supported_pgtb_type(pgtb_ent->type)) {
		if ((ret = create_pgtb_entry(pgd_ent->phy_addr, pgtb_ent)) == ERR_SUCC) {
		    ret = create_pgd_entry(pgdir, pgd_ent);
		}
	    } else {
		ret = ERR_NOTSUPP;
	    }
	} else {
	    ret = ERR_INVAL;
	}
    } else {
	ret = ERR_INVAL;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_pgd_pgtb
 * 
 * creates both a new page directory and page table entry based on specified structures.
 * this requires the mmu is initialized prior to calls to this function.
 * this (logically) requires that pgd_ent->phy_addr is the page table
 * and pgd_ent->virt_addr == pgtb_ent->virt_addr.
 * 
 * @pgd_ent	page directory entry
 * @pgtb_ent	page table entry
 * @return errno
 **/
int armv7_mmu_map_pgd_pgtb(struct armv7_mmu_pgd_entry *pgd_ent, struct armv7_mmu_pgtb_entry *pgtb_ent) {
    addr_t	pgdir	= 0;
    int		ret	= ERR_SUCC;
    
    if (init) {
	if (pgd_ent != NULL && pgtb_ent != NULL) {
	    if (pgd_ent->type == ARMV7_MMU_PGD_TABLE) {
		if (armv7_is_supported_pgtb_type(pgtb_ent->type)) {
		    if (is_higher_half(pgd_ent->virt_addr, pg_div)) {
			pgdir = kern_pgd;
		    } else {
			pgdir = user_pgd;
		    }
		    
		    if ((ret = create_pgtb_entry(pgd_ent->virt_addr, pgtb_ent)) == ERR_SUCC) {
			ret = create_pgd_entry(pgdir, pgd_ent);
			
			if ((ret == ERR_SUCC) && armv7_is_mmu_enabled()) {
			    armv7_mmu_update();
			}
		    }
		} else {
		    ret = ERR_NOTSUPP;
		}
	    } else {
		ret = ERR_INVAL;
	    }
	} else {
	    ret = ERR_INVAL;
	}
    } else {
	ret = ERR_NOTINIT;
    }
    
    return ret;
}

/**
 * armv7_mmu_map_new_pgtb
 * 
 * creates a page table entry from specified page directory.
 * this requires that a prior page directory entry was created prior to
 * calls to this function.
 * this does not (and must not) effect the current operating state of the mmu
 * 
 * @pgd		address of specified page directory
 * @pgtb_ent	page table entry to create
 * @return errno
 **/
int armv7_mmu_map_new_pgtb(addr_t pgd, struct armv7_mmu_pgtb_entry *pgtb_ent) {
    int ret = ERR_SUCC;
    
    if (pgtb_ent != NULL) {
	if (armv7_is_supported_pgtb_type(pgtb_ent->type)) {
	    struct armv7_mmu_pgd_entry pgd_ent;
	
	    if ((ret = get_pgd_entry(pgd, pgtb_ent->virt_addr, &pgd_ent)) == ERR_SUCC) {
		if (pgd_ent.type == ARMV7_MMU_PGD_TABLE) {
		    ret = create_pgtb_entry(pgd_ent.phy_addr, pgtb_ent);
		} else {
		    ret = ERR_NOTFND;
		}
	    } else {
		ret = ERR_NOTSUPP;
	    }
	}
    } else {
	ret = ERR_INVAL;
    }
    
    return ret;
}

/**
 * make_ttbr0_mask
 * 
 * creates and returns the ttbr0 mask based on the pg_div
 * @n	page div
 * @return ttbr0 mask
 **/
static unsigned int make_ttbr0_mask(int n) {
    unsigned int ret = 0;
    
    for (int i = (14 - n); i < 31; i++) {
	ret |= (1 << i);
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
 * causes the mmu to update to changes 
 **/
static void armv7_mmu_update(void) {
    /* make sure everything is written */
    dsb();
    
    /* invalidate tlb */
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
    int			ret	= ERR_SUCC;
    
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
	ret = ERR_INVAL;
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
    int			ret	= ERR_SUCC;
    
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
	ret = ERR_INVAL;
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
	    ret = ERR_NOTFND;
	}
    } else {
	ret = ERR_INVAL;
    }
    
    return ret;
}
    
    
