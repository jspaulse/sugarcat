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

#include <arch/arm/armv7/armv7_syscntl.h>
#include <arch/arm/cpu/cortex_a9.h>
#include <mach/vexpress_a9/vexpress_a9.h>
#include <mach/mach_init.h>
#include <init/kinit.h>
#include <util/atag.h>
#include <mm/mm.h>
#include <types.h>
#include <errno.h>
#include <stdbool.h>

#define MB 				0x100000
#define MMU_PTE_SIZE	0x200000
#define MASK_MB 		0xFFFFF
#define DIV_MULT_MB 	20

/* used for kinit_warn/info/print */
static char buf[512];

/* helper functions */
static int init_get_mem(addr_t atag_base, struct mm_reg *reg);
static int init_get_initrd(addr_t atag_base, struct mm_reg *reg);

/**
 * vexpress_init
 * 
 * performs actual initializations of things required by the kernel 
 * and eventually branches into the kernel initialization
 * 
 * (TODO) describe in light detail what we're doing
 **/
void vexpress_init(unsigned int mach, addr_t atag_base) {
	struct mm_reg	kern_reg	= {init_kvm_to_phy((addr_t)&k_start), ((size_t)&k_end - (size_t)&k_start)};
	struct mm_reg	mmu_pte_reg	= {(init_kvm_to_phy((addr_t)&k_end) & ~MASK_MB) + MB, MMU_PTE_SIZE};
	//struct mm_reg	mmu_pgd_reg	= {(addr_t)&k_pgd, 0x4000};
	struct mm_reg	initrd_reg	= {0, 0};
	struct mm_reg	mem_reg 	= {0, 0};
	bool			initrd_ex	= false;
	int				err			=0;
	
	/*
	struct mm_reg	*reserved_regs[] = {
		&kern_reg,
		&mem_reg,
		&initrd
	};
	*/

	/* grab the largest continuous region of memory */
	if ((err = init_get_mem(atag_base, &mem_reg)) != ERR_SUCC) {
		kinit_panic(buf, "init_get_mem() returned %i, no defined memory regions available.", err);
	} else if (mem_reg.size <= (kern_reg.size + mmu_pte_reg.size)) {
		kinit_panic(buf, "not enough memory in region!  init_get_mem() returned %i bytes \
			in largest found region, a minimum of %i bytes are required.", mem_reg.size, kern_reg.size + mmu_pte_reg.size);
	}
	
	/* check if initrd exists */
	if (tag_exists(atag_base, ATAG_INITRD2)) {
		if ((err = init_get_initrd(atag_base, &initrd_reg)) != 0) {
			kinit_warn(buf, "tag_exists(ATAG_INITRD2) returned true but init_get_initrd() returned %i; \
				assuming it doesn't exist.", err);
			initrd_ex = false;
		} else if (initrd_reg.size == 0) {
			kinit_warn(buf, "ATAG_INITRD2 exists but reports initrd.size == %i; \
				assuming it doesn't exist.", initrd_reg.size);
			initrd_ex = false;
		} else {
			initrd_ex = true;
		}
	}
	
	/* sanity check on reservations */
	if (!is_within_region(&mem_reg, &kern_reg)) {
		kinit_warn(buf, "is_within_region() reports kernel region is not in largest found memory region; \
			k_base: 0x%x, k_size: %i, m_base: 0x%x, m_size: %i.", kern_reg.base, kern_reg.size, mem_reg.base, mem_reg.size);
	} 
	
	/* check if mmu pte is in sane region */
	if (!is_within_region(&mem_reg, &mmu_pte_reg)) {
		kinit_warn(buf, "is_within_region() reports mmu_pte region is not in largest found memory region; \
			pte_base: 0x%x, pte_size: %i, m_base: 0x%x, m_size: %i.", mmu_pte_reg.base, mmu_pte_reg.size, mem_reg.base, mem_reg.size);
	}
	
	/* check if overlapping with initrd */
	if (initrd_ex && is_overlapping(&initrd_reg, &mmu_pte_reg)) {
		kinit_warn(buf, "is_overlapping() reports mmu_pte is overlapping the initrd; pte_base: 0x%x, pte_size: %i, \
			initrd_base: 0x%x, initrd_size: %i.", mmu_pte_reg.base, mmu_pte_reg.size, initrd_reg.base, initrd_reg.size);
	}
	
	/* do preliminary interrupt enabling */
	/* allocate stacks for exceptions */
	
	/* bring cpu's up into holding pen */
	/* (this can be arch/arm/armv7/cpus) */
	/* (this also means we'll need to define the private interrupts */
	/* for stuff and things) */
	
	/* stop complaining! */
	if (mach == 0) {
		
	}
	
	/* what we're going to do here:
	 * bring up full fledged mmu w/small(?) pages
	 *	this requires 2 MiB for the secondary tables
	 * interrupts - only enable irq #0 (for cpus)
	 * bring up secondary cpus and put them in holding pen
	 * 	secondary cpu's will need their mmu's set up and stacks
	 * since ss is going to be trashed, we'll need to keep the 
	 * stack page reserved.
	 * 
	 **/
	
	while(1);
}

/**
 * init_get_mem
 * 
 * returns the largest region of memory listed in the atags
 * 
 * this will attempt to coalesce any adjacent memory regions
 * 
 * @atag_base	atag base address
 * @reg			struct to output into
 * @return errno
 **/
static int init_get_mem(addr_t atag_base, struct mm_reg *reg) {
	struct atag 	*sch	= NULL;
	struct mm_reg	fnd 	= {0, 0};
	int 			ret 	= ERR_SUCC;

	if (reg != NULL) {
		sch = get_tag(atag_base, ATAG_MEM);
		
		if (sch != NULL) {
			fnd.base	= sch->u.mem.start;
			fnd.size	= sch->u.mem.sz;
			
			while ((sch = get_next_tag(sch, ATAG_MEM)) != NULL) {
				if ((fnd.base + fnd.size) == sch->u.mem.start) { /* regions are continuous */
					fnd.size += sch->u.mem.sz;
				} else if (fnd.size < sch->u.mem.sz) { /* larger region */
					fnd.base	= sch->u.mem.start;
					fnd.size	= sch->u.mem.sz;
				}
			}
			
			memcpy(reg, &fnd, sizeof(struct mm_reg));
		
		} else {
			ret = ERR_NOTFND;
		}
	} else {
		ret = ERR_INVAL;
	}
	
	return ret;
}

/**
 * init_get_initrd
 * 
 * returns the initrd memory region (if found)
 * 
 * @atag_base	atag base address
 * @reg			struct to output to
 * @return errno
 **/
static int init_get_initrd(addr_t atag_base, struct mm_reg *reg) {
	struct atag 	*sch	= NULL;
	int 			ret 	= 0;
	
	if (reg != NULL) {
		sch = get_tag(atag_base, ATAG_INITRD2);
		
		if (sch != NULL) {
			reg->base = sch->u.initrd.start;
			reg->size = sch->u.initrd.sz;
		} else {
			ret = ERR_NOTFND;
		}
	} else {
		ret = ERR_INVAL;
	}
	
	return ret;
}
