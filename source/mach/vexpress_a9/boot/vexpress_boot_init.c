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
#include <arch/arm/armv7/armv7_mmu.h>
#include <arch/arm/armv7/armv7_syscntl.h>
#include <arch/arm/armv7/armv7.h>
#include <memlayout.h>
#include <mm/mem.h>
#include <types.h>
#include <stddef.h>

#define KERN_PGD_ENTRY_CNT	2048
#define DIV_MULT_MB		20

/* init_mmu functions */
static void init_user_pg_dir(addr_t u_phy_pg_dir);
static void init_kern_pg_dir(addr_t k_phy_pg_dir, addr_t k_phy_start, size_t k_sz);
static void init_pg_dir_entry(addr_t *pg_dir, addr_t phy_addr, addr_t virt_addr);
static void init_enable_mmu(void);

/* TODO: tmp */
extern void kernel_init(unsigned int, addr_t, void *, void *, int);
//extern void kernel_init(unsigned int mach, addr_t atag_fdt_base, struct mm_vreg *mmu_pgtb_reg, struct mm_vreg *reserved_regs, int reg_cnt);

/**
 * vexpress_boot_init
 * 
 * initial C entry from vexpress_boot.s, initializes
 * mmu to move sp to high memory and branch into actual initialization
 * in vexpress_init
 **/
void vexpress_boot_init(unsigned int r0, int mach, addr_t atag_fdt_base) {
    size_t bss_sz 	= (size_t)&lmi_bss_end - (size_t)&lmi_bss_start;
    size_t k_sz		= (size_t)kvm_to_phy((addr_t)&k_end) - (size_t)&lmi_start;
    
    /* clean bss */
    memset(&lmi_bss_start, 0, bss_sz);
	
    /*
     * r0 is supposed to be 0, but not so sure
     * we care enough to actually do anything
     * if it isn't.
     */
    if (r0 != 0) {
	/* should we do something here? */
    }
    
    /* init/enable mmu */
    init_user_pg_dir((addr_t)&k_pgd);
    init_kern_pg_dir((addr_t)&k_pgd, (addr_t)&lmi_start, k_sz);
    init_enable_mmu();
    
    /* branch to main initialization code */
    //vexpress_init(mach, atag_dt_base);
    kernel_init(mach, atag_fdt_base, NULL, NULL, 0);
    //void kernel_init(unsigned int mach, addr_t atag_fdt_base, 
    //struct mm_vreg *mmu_pgtb_reg, struct mm_vreg *reserved_regs, 
    //int reg_cnt) {
}

/**
 * init_enable_mmu
 * 
 * initializes and enables the mmu
 * 
 **/
static void init_enable_mmu(void) {
    unsigned int reg = 0;
    
    /* set the mmu split */
    armv7_set_ttbcr(ARMV7_TTBCR_2G_2G);
	
    /* set domains */
    armv7_set_domain(0, ARMV7_DACR_MNGR);
	
    /* set the control bits and enable mmu */
    reg = armv7_get_sctlr();
    reg |= ARMV7_SCTLR_AFE | ARMV7_SCTLR_MMU_ENB;
    armv7_set_sctlr(reg);
	
    /* invalidate */
    armv7_invalidate_unified_tlb();
}
	

/**
 * init_user_pg_dir
 * 
 * initializes the user (TTB0) page dir as a 1:1 mapping of
 * all physical addresses from [0, 2GiB]
 * 
 * @u_pg_dir	physical address of the user page dir
 **/
static void init_user_pg_dir(addr_t u_phy_pg_dir) {
    addr_t *pg_dir = (addr_t *)u_phy_pg_dir;

	
    /* map all in 2GiB range */
    for (int i = 0; i < KERN_PGD_ENTRY_CNT; i++) {
	addr_t pv_addr = (i << DIV_MULT_MB);
		
	/* map 1:1 */
	init_pg_dir_entry(pg_dir, pv_addr, pv_addr);
    }
	
    dsb();
    armv7_set_ttbr0(u_phy_pg_dir);
}

/**
 * init_kern_pg_dir
 * 
 * initializes the kernel (TTB1) page dir as k_phy_mem -> k_virt_mem
 * 
 * @k_phy_pg_dir	physical address of the kernel page dir
 * @k_phy_start		physical address of the start of the kernel
 * @k_sz		size of the kernel
 **/
static void init_kern_pg_dir(addr_t k_phy_pg_dir, addr_t k_phy_start, size_t k_sz) {
    addr_t *pg_dir 	= (addr_t *)k_phy_pg_dir;
    int cnt 		= (k_sz >> DIV_MULT_MB) + 1;	/* number of 1MiB sections to map */
	
    /* map all MiB sections of kernel */
    for (int i = 0; i < cnt; i++) {
	addr_t p_addr = k_phy_start + (i << DIV_MULT_MB);
		
	/* map kernel sections in high mem */
	init_pg_dir_entry(pg_dir, p_addr, phy_to_kvm(p_addr));
    }
	
    dsb();
    armv7_set_ttbr1(k_phy_pg_dir);
}

/**
 * init_pg_dir_entry
 * 
 * creates a section entry in the specified pg_dir for the specified physical
 * and virtual addresses
 * 
 * @pg_dir	pointer to page directory
 * @phy_addr	physical address being mapped to virtual address
 * @virt_addr	virtual address being mapped to physical address
 **/
static void init_pg_dir_entry(addr_t *pg_dir, addr_t phy_addr, addr_t virt_addr) {
    unsigned int entry = (phy_addr & PGD_SECT_MASK) | (ARMV7_MMU_ACC_KRW_URW << 10) | ARMV7_MMU_PGD_SECTION;

    pg_dir[(virt_addr >> DIV_MULT_MB)] = entry;
}

