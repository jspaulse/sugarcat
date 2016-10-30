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
#include <mm/mmu.h>
//#include <mm/mem.h>
#include <mm/pmm.h>
#include <util/bits.h>
#include <util/atag.h>
#include <kmap.h>
#include <types.h>

extern void d_printf(const char *format, ...); //tmp
static int setup_mmu(addr_t kpgd, addr_t upgd);

void initsys(struct mm_region k_phy, struct mm_region phy_kpgd, struct mm_region phy_upgd, addr_t atags) {
	addr_t vir_start		= mmu_get_high_vaddr(kmap_mvm);
	size_t bss_sz			= (size_t)&bss_end - (size_t)&bss_start;
	struct mm_region kpgd 	= { phy_kpgd.start + vir_start, phy_kpgd.size };
	struct mm_region upgd 	= { phy_upgd.start + vir_start, phy_upgd.size };
	addr_t bm_vir_start 	= (((addr_t)&k_end >> DIV_MB) + 1) << DIV_MB;
	addr_t bm_phy_start		= bm_vir_start - vir_start;
	int err					= 0;
	struct atag atg;

	/* clear el bss'o! */
	memset(&bss_start, 0, bss_sz);
	
	/* find memory tag */
	if ((err = find_tag(atags, ATAG_MEM, &atg)) == 0) {
		if ((err = setup_mmu(kpgd.start, upgd.start)) == MMU_ERR_SUCCESS) {
			/* write vmm
			 * map physical region (should be ~2MB)
			 * allocate virtual addresses for PMM
			 * initialize PMM
			 */
			
			d_printf("k_phy: 0x%x\n", k_phy.start);
			d_printf("bm_vir_start: 0x%x\n", bm_vir_start);
			d_printf("bm_phy_start: 0x%x\n", bm_phy_start);
			d_printf("did we survive?\n");
		} else {
			/* throw another fit */
		}
	} else {
		/* fit throw on atags */
	}
	
	while (true) {
		/* do something */
	}
}

static int setup_mmu(addr_t kpgd, addr_t upgd) {
	int ret = 0;
	
	if (kpgd == 0) {
		
	} else if (upgd == 0) {
		
	}
	/*
	if (mmu_set_vm_map(kmap_mvm, true) == 0) {
		ret = mmu_set_pg_dirs(kpgd, upgd, true);
	}
	
	*/
	
	return ret;
}

/*
static int setup_map_bitmap(struct mm_region *bm_vir, addr_t bm_vir_start, size_t mem_sz) {
	int ret = 0;
	addr_t vir_start = mmu_get_high_vaddr(kmap_mvm);
	
	bm_vir->start 	= bm_vir_start;
	bm_vir->size	= bitmap_oh_pg_cnt(mem_sz) * PAGE_SZ;
	
	return mmu_map(bm_vir->start, 
	
	bm_vir
	unsigned int oh_pg_cnt = bitmap_oh_pg_cnt(atg.u.mem.sz);
			struct mm_region bm_vir = { bm_vir_start, oh_pg_cnt };

			err = mmu_map(bm_vir.start, bm_phy_start, PG_DIR_ENTRY, KERN_RW, 0, true);
			
			if (err == MMU_ERR_SUCCESS) {
				
			} else {
				
			}
*/
