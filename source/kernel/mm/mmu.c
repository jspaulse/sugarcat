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
#include <arch/arch_mmu.h>
#include <mm/mmu.h>
#include <types.h>
#include <stdbool.h>

static addr_t kern_pgd		= 0;
static addr_t user_pgd		= 0;
static mmu_vm_map mmu_mvm	= 0;

/* prototypes */
static int _mmu_map(addr_t pg_dir, addr_t vaddr, addr_t paddr, pg_ent_t ent, pg_acc_t acc, unsigned char flags);

int mmu_set_vm_map(mmu_vm_map mvm, bool update) {
	int ret = MMU_ERR_SUCCESS;
	
	if (update) {
		if (arch_is_mmu_enabled()) {
			ret = MMU_ERR_ALREADY_ENABLED;
		} else {
			ret = arch_mmu_set_vm_map(mvm);
		}
	}
	
	mmu_mvm = mvm;

	return ret;
}

int mmu_enable(void) {
	int ret = MMU_ERR_ALREADY_ENABLED;
	
	if (!arch_is_mmu_enabled()) {
		ret = arch_mmu_enable();
	}
	
	return ret;
}

int mmu_disable(void) {
	int ret = MMU_ERR_NOT_ENABLED;
	
	if (arch_is_mmu_enabled()) {
		ret = arch_mmu_disable();
	}

	return ret;
}

int mmu_set_user_pg_dir(addr_t pgd_addr, bool invalidate) {
	int ret = MMU_ERR_SUCCESS;

	if ((ret = arch_mmu_set_user_pg_dir(pgd_addr)) == MMU_ERR_SUCCESS) {
		user_pgd = pgd_addr;
		
		if (invalidate) {
			arch_mmu_invalidate_pg_dirs();
		}
	}
	
	return ret;
}

int mmu_set_kern_pg_dir(addr_t pgd_addr, bool invalidate) {
	int ret = MMU_ERR_SUCCESS;
	
	if ((ret = arch_mmu_set_kern_pg_dir(pgd_addr)) == MMU_ERR_SUCCESS) {
		kern_pgd = pgd_addr;
		
		if (invalidate) {
			arch_mmu_invalidate_pg_dirs();
		}
	}
	
	return ret;
}

int mmu_set_pg_dirs(addr_t kpgd, addr_t upgd, bool invalidate) {
	int ret = MMU_ERR_SUCCESS;
	
	if ((ret = mmu_set_user_pg_dir(upgd, invalidate)) == MMU_ERR_SUCCESS) {
		ret = mmu_set_kern_pg_dir(kpgd, invalidate);
	}

	return ret;
}

static int _mmu_map(addr_t pg_dir, addr_t vaddr, addr_t paddr, pg_ent_t ent, pg_acc_t acc, unsigned char flags) {
	struct mmu_entry mme = {
		.pg_dir	= pg_dir,
		.vaddr	= vaddr,
		.paddr	= paddr,
		.entry	= ent,
		.access	= acc,
		.flags	= flags
	};
	
	return arch_mmu_map(&mme);
}

int mmu_map_new(addr_t pg_dir, addr_t vaddr, addr_t paddr, pg_ent_t ent, pg_acc_t acc, unsigned char flags) {
	return _mmu_map(pg_dir, vaddr, paddr, ent, acc, flags);
}

int mmu_map(addr_t vaddr, addr_t paddr, pg_ent_t ent, pg_acc_t acc, unsigned char flags, bool invalidate) {
	int ret = 0, err = 0;
	addr_t pgd;
	
	if (kern_pgd == 0 || user_pgd == 0) {
		ret = MMU_ERR_NO_PAGE_DIRS;
	} else {
		if (!arch_is_mmu_enabled()) {
			ret = MMU_ERR_NOT_ENABLED;
		} else {
			if (vaddr >= mmu_get_high_vaddr(mmu_mvm)) {
				pgd = kern_pgd;
			} else {
				pgd = user_pgd;
			}
			
			err = _mmu_map(pgd, vaddr, paddr, ent, acc, flags);
			
			if (err == MMU_ERR_SUCCESS) {
				if (invalidate) {
					arch_mmu_invalidate_pg_dirs();
				}
			} else {
				ret = err;
			}
		}
	}
	
	return ret;
}
