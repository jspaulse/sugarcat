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
 * 
 * armv7_arch_mmu.c provides the arch_mmu_interface.
 */
#include <arch/arm/armv7/armv7_syscntl.h>
#include <arch/arm/armv7/armv7_mmu.h>
#include <arch/arch_mmu.h>
#include <mm/mmu.h>
#include <stdbool.h>

static armv7_mmu_acc_perm arch_mmu_acc_to_armv7(mmu_acc_flags_t flags);
static armv7_mmu_pgd_type arch_mmu_pgd_type_to_armv7(mmu_entry_type_t type);
static armv7_mmu_pgtb_type arch_mmu_pgtb_type_to_armv7(mmu_entry_type_t type);
static unsigned char arch_mmu_acc_to_domain(mmu_acc_flags_t flags);

bool arch_mmu_is_enabled(void) {
    return armv7_mmu_is_enabled();
}

addr_t virt_to_phy(addr_t virt_addr) {
    return armv7_mmu_virt_to_phy(virt_addr);
}

int arch_mmu_set_user_pg_dir(addr_t page_dir) {
    return armv7_mmu_set_user_pgd(page_dir, 0x0);
}

int arch_mmu_create_entry(struct mmu_entry *entry) {
    struct armv7_mmu_pgd_entry 	armv7_pgd_entry;
    struct armv7_mmu_pgtb_entry armv7_pgtb_entry;
    int				ret	= 0;
    
    /* create based on type */
    switch (entry->type) {
	case PG_DIR_INVAL:
	case PG_DIR:
	    armv7_pgd_entry.phy_addr	= entry->phy_addr;
	    armv7_pgd_entry.virt_addr	= entry->virt_addr;
	    armv7_pgd_entry.domain	= arch_mmu_acc_to_domain(entry->acc_flags);
	    armv7_pgd_entry.type	= arch_mmu_pgd_type_to_armv7(entry->type);
	    armv7_pgd_entry.flags	= 0x0;	/* TODO */
	    
	    ret = armv7_mmu_map_pgd(&armv7_pgd_entry);
	    break;
	case PG_TAB_INVAL:
	case PG_TAB:
	    armv7_pgtb_entry.phy_addr	= entry->phy_addr;
	    armv7_pgtb_entry.virt_addr	= entry->virt_addr;
	    armv7_pgtb_entry.acc_perm	= arch_mmu_acc_to_armv7(entry->acc_flags);
	    armv7_pgtb_entry.type	= arch_mmu_pgtb_type_to_armv7(entry->type);
	    armv7_pgtb_entry.flags	= 0x0;	/* TODO */
	    
	    ret = armv7_mmu_map_pgtb(&armv7_pgtb_entry);
	    break;
    }
    
    return ret;
}

int arch_mmu_create_new_entry(addr_t pg_base, struct mmu_entry *entry) {
    struct armv7_mmu_pgd_entry 	armv7_pgd_entry;
    struct armv7_mmu_pgtb_entry armv7_pgtb_entry;
    addr_t			kvaddr		= 0x0;
    int				index		= 0;
    int				ret		= 0;
    
    /* create based on type */
    switch (entry->type) {
	case PG_DIR_INVAL:
	case PG_DIR:
	    armv7_pgd_entry.phy_addr	= entry->phy_addr;
	    armv7_pgd_entry.virt_addr	= entry->virt_addr;
	    armv7_pgd_entry.domain	= arch_mmu_acc_to_domain(entry->acc_flags);
	    armv7_pgd_entry.type	= arch_mmu_pgd_type_to_armv7(entry->type);
	    armv7_pgd_entry.flags	= 0x0;	/* TODO */
	    
	    ret = armv7_mmu_map_new_pgd(pg_base, &armv7_pgd_entry);
	    break;
	case PG_TAB_INVAL:
	case PG_TAB:
	    armv7_pgtb_entry.phy_addr	= entry->phy_addr;
	    armv7_pgtb_entry.virt_addr	= entry->virt_addr;
	    armv7_pgtb_entry.acc_perm	= arch_mmu_acc_to_armv7(entry->acc_flags);
	    armv7_pgtb_entry.type	= arch_mmu_pgtb_type_to_armv7(entry->type);
	    armv7_pgtb_entry.flags	= 0x0;	/* TODO */
	    
	    /* grab kvaddr */
	    kvaddr = arch_mmu_get_kern_vaddr();
	    
	    /* determine if this is high or low mem */
	    if (entry->virt_addr >= kvaddr) {
		index = ((entry->virt_addr - kvaddr) >> PGD_IDX_SHIFT);
	    } else {
		index = (entry->virt_addr >> PGD_IDX_SHIFT);
	    }
	    
	    ret = armv7_mmu_map_new_pgtb((pg_base | (index << PG_IDX_SHIFT)), &armv7_pgtb_entry);
	    break;
    }
    
    return ret;
}

size_t arch_mmu_get_user_pgtb_reg_sz(void) {
    int		pg_div	= armv7_mmu_get_pg_div();
    size_t	ret	= 0;	
    
    if (pg_div > 0) {
	ret = (1 << (32 - (pg_div + PGD_IDX_SHIFT))) * PGTB_SZ;
    } else {
	ret = PGD_ENTRY_CNT * PGTB_SZ;
    }
    
    return ret;
}

addr_t arch_mmu_get_kern_vaddr(void) {
    int		pg_div	= armv7_mmu_get_pg_div();
    addr_t 	ret 	= 0x0;
    
    if (pg_div > 0) {
	ret = (1 << (32 - pg_div));
    } else {
	ret = 0x0;
    }
    
    return ret;
}

size_t arch_mmu_get_user_pgd_sz(void) {
    int		pg_div	= armv7_mmu_get_pg_div();
    size_t	ret	= 0x0;
    
    if (pg_div > 0) {
	ret = (1 << (32 - (pg_div + PGD_IDX_SHIFT))) * PGD_ENTRY_SZ;
    } else {
	ret = PGD_ENTRY_CNT * PGD_ENTRY_SZ;
    }
    
    return ret;
}

bool arch_mmu_user_pgd_requires_alignment(void) {
    return true;
}

unsigned int arch_mmu_get_user_pgd_alignment(void) {
    return (1 << (TTBR_ALIGN - armv7_mmu_get_pg_div()));
}

void arch_mmu_invalidate(void) {
    armv7_invalidate_unified_tlb();
}

/**
 * arch_mmu_acc_to_domain
 * 
 * translates mmu access flags to domain
 * @flags	mmu access flags
 * @return domain
 **/
static unsigned char arch_mmu_acc_to_domain(mmu_acc_flags_t flags) {
    unsigned char ret = 0;
    
    switch (flags) {
	case USER:
	case KERN_USER:
	    ret = USER_DOMAIN;
	    break;
	case KERNEL_RO:
	case KERNEL:
	case DEVICE:
	    ret = KERN_DOMAIN;
	    break;
    }
    
    return ret;
}

/**
 * arch_mmu_acc_to_armv7
 * 
 * translates between mmu & armv7 access types
 * @flags	mmu access flags
 * @return armv7 access flags
 **/
static armv7_mmu_acc_perm arch_mmu_acc_to_armv7(mmu_acc_flags_t flags) {
    armv7_mmu_acc_perm ret = 0;
    
    switch (flags) {
	case USER:
	    ret = ARMV7_MMU_ACC_KRW_URW;
	    break;
	case KERN_USER:
	    ret = ARMV7_MMU_ACC_KRO_URO;
	    break;
	case KERNEL:
	    ret = ARMV7_MMU_ACC_KRW_NOU;
	    break;
	case KERNEL_RO:
	    ret = ARMV7_MMU_ACC_KRO_NOU;
	    break;
	case DEVICE:
	    ret = ARMV7_MMU_ACC_KRW_NOU;
	    break;
    }
    
    return ret;
}

/**
 * arch_mmu_pgtb_type_to_armv7
 * 
 * translates between mmu & armv7 pgtb entry types
 * @type	mmu entry type
 * @return armv7 pgtb entry type
 **/
static armv7_mmu_pgtb_type arch_mmu_pgtb_type_to_armv7(mmu_entry_type_t type) {
    armv7_mmu_pgtb_type ret;
    
    switch (type) {
	case PG_TAB_INVAL:
	    ret = ARMV7_MMU_PGTB_INVALID;
	    break;
	case PG_TAB:
	    ret = ARMV7_MMU_PGTB_SMALL_PG;
	    break;
	case PG_DIR_INVAL:
	case PG_DIR:
	    ret = ARMV7_MMU_PGTB_INVALID;
	    break;
    }
    
    return ret;
}

/**
 * arch_mmu_pgd_type_to_armv7
 * 
 * translates between mmu & armv7 pgd entry types
 * @type	mmu entry type
 * @return armv7 pgd entry type
 **/
static armv7_mmu_pgd_type arch_mmu_pgd_type_to_armv7(mmu_entry_type_t type) {
    armv7_mmu_pgd_type ret;
    
    switch (type) {
	case PG_DIR_INVAL:
	    ret = ARMV7_MMU_PGD_INVALID;
	    break;
	case PG_DIR:
	    ret = ARMV7_MMU_PGD_TABLE;
	    break;
	case PG_TAB_INVAL:
	case PG_TAB:
	    ret = ARMV7_MMU_PGD_INVALID;
	    break;
    }
    
    return ret;
}
