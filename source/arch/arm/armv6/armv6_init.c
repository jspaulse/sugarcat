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
#include <mm/mem.h>
#include <util/bits.h>
#include <kmap.h>

static unsigned int v_start = (unsigned int)&kv_start;
extern void add_sp(unsigned int add_sp);

extern void set_sp(unsigned int address);
void initsys(struct mm_region k_phy, struct mm_region phy_kpgd, struct mm_region phy_upgd, addr_t atags);
extern void d_printf(const char *format, ...); //tmp
#define UART0_BASE_ADDRESS	0x20200000 // tmp 

static void ei_mmu_entry(unsigned int paddr, unsigned int vaddr);
static void ei_mmu_enable(void);

void early_init(unsigned int r0, unsigned int mach_type, unsigned int atags) {
	size_t ss_bss_sz	= (size_t)&ss_bss_end - (size_t)&ss_bss_start;
	addr_t k_phy_start	= (addr_t)&k_start - (addr_t)&kv_start;
	size_t k_sz			= (size_t)&k_end - (size_t)&k_start;
	addr_t sts_start	= (addr_t)&ss_start;
	size_t sts_sz		= (size_t)&ss_end - (size_t)&ss_start;
	
	/* pass to initsys */
	struct mm_region k_phy_reg 	= {k_phy_start, k_sz};
	struct mm_region kpgd_reg 	= {MMU_KPGDIR, MMU_PGD_SZ};
	struct mm_region upgd_reg 	= {MMU_UPGDIR, MMU_PGD_SZ};
	
	/* clear el bss'o */
	memset(&ss_bss_start, 0, ss_bss_sz);
	
	if (r0 != 0) {
		/* OH MAH GAWD! */
	}
	
	if (mach_type == 0) {
		/* who cares? */
	}
	
	/* start mapping junk! wooo! */
	/* map start sec region */
	/* check if both fall under the same MB region? */
	for (unsigned int i = sts_start; i < (sts_start + sts_sz); i+= 0x100000) {
		ei_mmu_entry(i, i);
	}
	
	/* map both this region and kernel region */
	for (unsigned int i = k_phy_start; i < (k_phy_start + k_sz); i += 0x100000) {
		ei_mmu_entry(i, i);
		ei_mmu_entry(i, v_start + i);
	}
	
	d_printf("u/kpgdir: 0x%x 0x%x\n", upgd_reg.start, kpgd_reg.start);
	
	ei_mmu_entry(UART0_BASE_ADDRESS, UART0_BASE_ADDRESS); /* tmp */

	/* now, enable! */
	ei_mmu_enable();
	
	/* set the stack to sp + v_start */
	add_sp(v_start);
	
	/* branch into the kernel initsys */
	initsys(k_phy_reg, kpgd_reg, upgd_reg, atags);
}

/* setting up the initial mappings */
static void ei_mmu_entry(unsigned int paddr, unsigned int vaddr) {
	unsigned int pde = ARMV6_MMU_SECTION | (ARMV6_MMU_ACC_URW << 10);
	unsigned int *pg_dir = NULL;
	int index = 0;
	
	if (vaddr >= v_start) {
		index = (vaddr - v_start) >> 20;
		pg_dir = (unsigned int *)MMU_KPGDIR;
	} else {
		index = vaddr >> 20;
		pg_dir = (unsigned int *)MMU_UPGDIR;
	}
	
	pg_dir[index] = pde | (paddr & 0xFFF00000);
}

static void ei_mmu_enable(void) {
	unsigned int val = 0;
	
	/* set domains (or lack of) */
    armv6_set_domain(ARMV6_NO_DOMAINS);
	
	/* set the mmu_vm_split */
	armv6_set_ttbcr(armv6_mmu_vm_map_to_arm(kmap_mvm));
	
    // set the kernel page table
    armv6_set_ttb1(MMU_KPGDIR);
	
    // set the user page table
    armv6_set_ttb0(MMU_UPGDIR);
	
	/* enable, configuration, OR current register */
	val = MMU_ENABLE | SUBPAGE_AP_DISABLED | armv6_read_cntl_reg();
	armv6_write_cntl_reg(val); 
    
    armv6_invalidate_tlbs();
}

