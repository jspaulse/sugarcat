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
#include <arch/arm/armv7/armv7_mmu.h>
#include <arch/arch_mmu.h>
#include <mm/mmu.h>
#include <stdbool.h>

static armv7_mmu_acc_perm arch_mmu_acc_to_armv7(mmu_acc_flags_t flags);

bool arch_mmu_is_enabled(void) {
    return armv7_mmu_is_enabled();
}

addr_t virt_to_phy(addr_t virt_addr) {
    return armv7_mmu_virt_to_phy(virt_addr);
}

int arch_mmu_set_user_pg_dir(addr_t page_dir) {
    return armv7_mmu_set_user_pgd(page_dir, 0x0);
}

int arch_mmu_create_pgtb_entry(struct mmu_entry *entry) {
    struct armv7_mmu_pgtb_entry armv7_entry = {
	.phy_addr	= entry->phy_addr,
	.virt_addr	= entry->virt_addr,
	.acc_perm	= arch_mmu_acc_to_armv7(entry->acc_flags),
	.type		= ARMV7_MMU_PGTB_SMALL_PG,
	.flags		= 0x0	/* TODO */
    };
    
    return armv7_mmu_map_pgtb(&armv7_entry);
}

int arch_mmu_create_pgd_entry(struct mmu_entry *entry) {
    struct armv7_mmu_pgd_entry 	armv7_entry = {
	.phy_addr	= entry->phy_addr,
	.virt_addr	= entry->virt_addr,
	.acc_perm	= arch_mmu_acc_to_armv7(entry->acc_flags),
	.type		= ARMV7_MMU_PGD_TABLE,
	.flags		= 0x0	/* TODO */
    };
    
    /* determine domain */
    if (entry->acc_flags == USER || entry->acc_flags == KERN_USER) {
	armv7_entry.domain = USER_DOMAIN;
    } else {
	armv7_entry.domain = KERN_DOMAIN;
    }
    
    return armv7_mmu_map_pgd(&armv7_entry);
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

/*
addr_t armv7_mmu_get_user_pgd(void);
addr_t armv7_mmu_get_kern_pgd(void);
int armv7_mmu_set_kern_pgd(addr_t pgd_addr, unsigned char flags);
int armv7_mmu_set_user_pgd(addr_t pgd_addr, unsigned char flags);

int armv7_mmu_map_pgd(struct armv7_mmu_pgd_entry *pgd_ent);
int armv7_mmu_map_pgtb(struct armv7_mmu_pgtb_entry *pgtb_ent);

int armv7_mmu_map_new_pgd(addr_t pgd_addr, struct armv7_mmu_pgd_entry *pgd_ent);
int armv7_mmu_map_new_pgtb(addr_t pgtb_addr, struct armv7_mmu_pgtb_entry *pgtb_ent);

addr_t armv7_mmu_virt_to_phy(addr_t virt_addr);
*/
