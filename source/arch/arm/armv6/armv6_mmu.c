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
#include <arch/arm/armv6_mmu.h>
#include <arch/arm/armv6.h>
#include <mm/mmu.h>
#include <types.h>
#include <stdbool.h>

#define PDE_SHIFT 		20
#define PDE_MASK		0xFFF00000
#define PDE_PTE_MASK	0xFFFFFC00
#define PDE_AP_SHIFT	10
#define PTE_SHIFT		12
#define PTE_MASK		0xFFFFF000
#define PTE_AP_SHIFT	4

static mmu_vm_map mmu_mvm = MMU_4G_NO_SPLIT;

static int armv6_map_pde(struct mmu_entry *mme);
static int armv6_map_pte(struct mmu_entry *mme);

extern void d_printf(const char *format, ...); // tmp


bool arch_is_mmu_enabled(void) {
	bool ret = false;
	
	if (armv6_read_cntl_reg() & MMU_ENABLE) {
		ret = true;
	}
	
	return ret;
}

int arch_mmu_map(struct mmu_entry *mme) {
	int ret = 0;
	
	if (mme->entry & PG_DIR_ENTRY) {
		ret = armv6_map_pde(mme);
	} else {
		ret = armv6_map_pte(mme);
	}
	
	return ret;
}

static int armv6_map_pde(struct mmu_entry *mme) {
	int ret = 0;
	int index 			= 0;
	unsigned int pde 	= 0;
	addr_t *pg_dir		= (addr_t *)mme->pg_dir;
	addr_t higher		= mmu_get_high_vaddr(mmu_mvm); 
	
	if (mme->vaddr >= higher) {
		index = (mme->vaddr - higher) >> PDE_SHIFT;
	} else {
		index = mme->vaddr >> PDE_SHIFT;
	}
	
	switch(mme->entry) {
		case PG_DIR_ENTRY:
			pde = (mme->paddr & PDE_MASK) | ARMV6_MMU_SECTION;
			pde |= armv6_pg_acc_to_arm(mme->access) << PDE_AP_SHIFT;
			break;
		case PG_DIR_TABLE:
			pde = (mme->paddr & PDE_PTE_MASK) | ARMV6_MMU_COARSE;
			break;
		case PG_DIR_TRANS_FAULT:
			pde = 0;
			break;
		default:
			break;
	}
	
	if (mme->flags & MMU_CACHED) {
		pde |= ARMV6_MMU_CACHED;
	}
	
	pg_dir[index] = pde;
	
	return ret;
}

static int armv6_map_pte(struct mmu_entry *mme) {
	int ret = 0;
	addr_t *pg_dir 		= (addr_t *)mme->pg_dir;
	addr_t *pg_table	= NULL; //(unsigned int *)(pg_dir[index] & 0xFFFFFC00);
	addr_t higher 		= mmu_get_high_vaddr(mmu_mvm);
	unsigned int pte	= 0;
	unsigned int index 	= 0;
	
	if (mme->vaddr >= higher) {
		index = (mme->vaddr - higher) >> PDE_SHIFT;
	} else {
		index = mme->vaddr >> PDE_SHIFT;
	}
	
	pg_table = (addr_t *)(pg_dir[index] & PDE_PTE_MASK);
	
	if (pg_table == NULL) {
		ret = MMU_ERR_NO_PAGE_TABLE;
	} else {
		index = (mme->vaddr >> PTE_SHIFT) & 0xFF;
		
		switch (mme->entry) {
			case PG_TABLE_ENTRY:
				pte = (mme->paddr & PTE_MASK) | ARMV6_MMU_4KB;
				pte |= armv6_pg_acc_to_arm(mme->access) << PTE_SHIFT;
				break;
			case PG_TABLE_TRANS_FAULT:
				pte = 0;
				break;
			default:
				break;
		}
		
		if (mme->flags & MMU_CACHED) {
			pte |= ARMV6_MMU_CACHED;
		}
		
		pg_table[index] = pte;
	}
	
	return ret;
}

int arch_mmu_set_vm_map(mmu_vm_map mvm) {
	int ret = 0;
	
	if (armv6_is_supported_vm_map(mvm)) {
		armv6_set_ttbcr(armv6_mmu_vm_map_to_arm(mvm));
		mmu_mvm = mvm;
	} else {
		ret = MMU_ERR_UNSUPPORTED_VM_MAP;
	}
	
	return ret;
}

int arch_mmu_set_user_pg_dir(addr_t pgd_addr) {
	int ret = 0;
	
	/* set ttb0 */
	armv6_set_ttb0(pgd_addr);
	
	return ret;
}

int arch_mmu_set_kern_pg_dir(addr_t pgd_addr) {
	int ret = 0;
	
	/* set ttb1 */
	armv6_set_ttb1(pgd_addr);

	return ret;
}

int arch_mmu_enable(void) {
	int ret = 0;
	unsigned int rd = 0;
	
	armv6_set_domain(0xFFFFFFFF);
	rd = armv6_read_cntl_reg() | (MMU_ENABLE | SUBPAGE_AP_DISABLED);
	armv6_write_cntl_reg(rd);

	return ret;
}

int arch_mmu_disable(void) {
	int ret = 0;
	unsigned int rd = armv6_read_cntl_reg();
	
	rd &= ~MMU_ENABLE;
	
	armv6_write_cntl_reg(rd);
	
	return ret;
}

void arch_mmu_invalidate_pg_dirs(void) {
	armv6_invalidate_tlbs();
}
