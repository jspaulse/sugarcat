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
#include <types.h>
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>

#define PG_DIV_MASK	0x7
#define DIV_MULT_MB	20

/* keep track of various things */
static bool init	= false;
static addr_t user_pgd 	= 0, kern_pgd = 0;
static int pg_div 	= 0;	/* mmu split */

/* various helpful things */
static unsigned int ttbr0_mask = 0;

/* helper functions */
static unsigned int make_ttbr0_mask(int n);
static bool is_higher_half(addr_t virt_addr, int n);

/**
 * armv7_mmu_set_pgd
 * 
 * sets the address of the page directory based on selection
 * 
 * @pgd_addr	base PHYSICAL address of page directory
 * @flags	associated flags for page directory
 * @pg_dir	selected page directory; this takes one of two options: KERN_PGD or USER_PGD
 * @return errno
 **/	
int armv7_mmu_set_pgd(addr_t pgd_addr, unsigned char flags, unsigned char pg_dir) {
    int ret = ERR_SUCC;
    
    if (pg_dir == KERN_PGD) {
	armv7_set_ttbr1(pgd_addr | flags);
	kern_pgd = pgd_addr;
    } else if (pg_dir == USER_PGD) {
	armv7_set_ttbr0(pgd_addr | flags);
	user_pgd = pgd_addr;
    } else {
	ret = ERR_INVAL;
    }
    
    /* update if we're enabled (and still valid) */
    if (armv7_is_mmu_enabled() && (ret == ERR_SUCC)) {
	dsb();
	
	armv7_invalidate_unified_tlb();
    }
    
    return ret;
}

/**
 * armv7_mmu_init_prior_enable
 * 
 * called whenever the mmu is enabled prior to init
 * but mmu needs to collect information
 * 
 * @return errno
 **/
int armv7_mmu_init_prior_enable(void) {
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
 * creates an entry on page directory based on virtual address
 * requires that the mmu be previously enabled
 * 
 * @paddr	physical address
 * @vaddr	virtual address
 * @domain	domain of entry
 * @acc_perm	access permissions of entry (should be zero for page table entry)
 * @type	type of entry
 * @flags	additional flags for entry
 * @update	whether or not the page directory should be updated
 * @return errno
 **/
int armv7_mmu_map_pgd(addr_t paddr, addr_t vaddr, unsigned char domain, unsigned char acc_perm, unsigned char type, unsigned char flags, bool update) {
    addr_t *pg_dir	= NULL;
    int ret 		= 0;
    
    if (init) {
	if (armv7_is_supported_pgd_type(type)) {
	    unsigned int index = vaddr >> DIV_MULT_MB;
	    unsigned int entry = 0;
	    
	    /* which table do we use? */
	    if (is_higher_half(vaddr, pg_div)) {
		pg_dir = (addr_t *)kern_pgd;
	    } else {
		pg_dir = (addr_t *)user_pgd;
	    }
	    
	    if (type == ARMV7_L1_PG_TB) {
		entry = paddr & PGD_TBL_MASK;
	    } else {
		entry = (paddr & PGD_SECT_MASK) | (acc_perm << ARMV7_L1_SECT_SHIFT_AP);
	    }
	    
	    entry |= (domain << ARMV7_L1_SHIFT_DOMAIN) | flags | type;
	    
	    /* finally, write it */
	    pg_dir[index] = entry;
	    
	    if (update) {
		dsb();
		armv7_invalidate_unified_tlb();
	    }
	} else {
	    ret = ERR_NOTSUPP;
	}
    } else {
	ret = ERR_NOTINIT;
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
 * @n		mmu split
 * @return true if higher
 **/
static bool is_higher_half(addr_t virt_addr, int n) {
    unsigned int 	higher 	= (1 << (32 - n));
    bool 		ret 	= false;

    if (virt_addr >= higher) {
	ret = true;
    }
    
    return ret;
}
    
    
    
    
