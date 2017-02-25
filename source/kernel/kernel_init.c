/* Copyright (C) 2017 Jacob Paulsen <jspaulse@ius.edu>
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
#include <mach/mach.h> /* TODO: tmp */
#include <init/kinit.h>
#include <mm/mem.h>
#include <types.h>
#include <util/fdt.h>
#include <memlayout.h>
#include <errno.h>


extern void install_ivt();
/**
 * expectation when entering kernel_init is that a 1:1 mapping has been
 * utilized.
 */
void kernel_init(unsigned int mach, addr_t atag_fdt_base, 
    struct mm_vreg *mmu_pgtb_reg, struct mm_vreg *reserved_regs, 
    int reg_cnt) {
    struct mm_reg mem_reg;
    size_t hmi_bss_sz	= (size_t)&hmi_bss_start - (size_t)&hmi_bss_end;
    size_t k_bss_sz	= (size_t)&k_bss_end - (size_t)&k_bss_start;
    
    /* clear hmi & kernel bss */
    memset(&hmi_bss_start, 0, hmi_bss_sz);
    memset(&k_bss_start, 0, k_bss_sz);
    
    mach_early_kprintf("inside kernel_init\n");
    install_ivt();
    
    dump_fdt(atag_fdt_base);
    
    if (mach) {
	if (atag_fdt_base) {
	    if (mmu_pgtb_reg) {
		if (reserved_regs) {
		    if (reg_cnt) {
			
		    }
		}
	    }
	}
    }
    
    int err = mlay_get_phy_mem_reg(atag_fdt_base, &mem_reg);
    if (err != ESUCC) {
	mach_early_kprintf("error: %i\n", err);
    } else {
	mach_early_kprintf("memory region: 0x%x, sized %i\n", mem_reg.base, 
	(mem_reg.size / 0x100000));
    }
    
    err = mlay_get_initrd_reg(atag_fdt_base, &mem_reg);
    
    if (err != ESUCC) {
	mach_early_kprintf("error: %i\n", err);
    } else {
	mach_early_kprintf("memory region: 0x%x, sized %i\n", mem_reg.base,
	    (mem_reg.size));
    }
    
    
    
    /* will need to map kernel hmi_init & hmi regions
     * as well as the k_stack */
/* what needs to be done here:
 * fix stack to high location
 * allocate aligned region for kernel pgtb
    * size & alignment can be grabbed from mmu (in arch currently)
    * zero out
 * fill in kernel pgtb for hmi_init/k_ regions
    * arch_mmu_map_new(p
 * map kpgd -> kpgtb
    * divide kernel region by page size into page count
    * and create entry per pg_cnt for the regions necessary.
 */

/*
 * mmu:
    Create functions for initializing/creating mmu_pgtbs
	All this requires is taking the kernel region, dividing
	    by page size (into page_cnt) & creating entry per
	    pg_cnt;
	
	    allocate space (1MiB above kernel, ALIGN_UP(1MiB Above kernel,
		arch_mmu_get_pgtb_align()));
	    arch_mmu_map_new(pgtbs, ...);
	    arch_mmu_map_new_pgtbs_pgdir(pgtbs, pgdir);
	keep track of this region, we'll need to pass to PMM
	*/
}


