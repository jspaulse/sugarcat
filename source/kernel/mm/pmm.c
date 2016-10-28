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
#include <stddef.h>
#include <mm/mem.h>
#include <mm/pmm.h>
#include <util/bits.h>

#define BM_OFFSET(pg_addr) 	((pg_addr >> DIV_PG) >> DIV_BITS)
#define BIT_OFFSET(pg_addr)	((pg_addr & BIT_MASK) >> DIV_PG)


static struct mm_region bm_region 	= {0, 0};
static unsigned int oh_pg_cnt		= 0;

extern void d_printf(const char *format, ...); //tmp
static void mark_allocated(addr_t pg_addr);
static void mark_allocated_cont(addr_t addr, int cnt);
//static void mark_unused(unsigned int pg_addr);
static unsigned char *get_address(addr_t pg_addr);
bool is_page_allocated(addr_t pg_addr);

int pmm_init_test(struct mm_region phy_mem, addr_t bm_phy_start, addr_t bm_vir_start, struct mm_region *used, int used_cnt) {
	int ret = 0;
	
	oh_pg_cnt = bitmap_oh_pg_cnt(phy_mem.size);
	
	/* make sure the region is at least (oh_pg_cnt + 1) * 4096 */
	if (((oh_pg_cnt + 1) << DIV_PG) <= phy_mem.size) {
		bm_region.start = bm_vir_start;
		bm_region.size	= bitmap_size(phy_mem.size);
		
		/* clear out bitmap region */
		memset((void *)bm_region.start, 0, bm_region.size);
		
		/* mark bitmap region allocated */
		mark_allocated_cont(bm_phy_start, oh_pg_cnt);
		
		/* now, allocate requested */
		for (int i = 0; i < used_cnt; i++) {
			mark_allocated_cont(used[i].start, mem_pg_cnt(used[i].size));
		}
	} else {
		ret = PMM_ERR_NOT_ENOUGH_MEM;
	}
		
	return ret;
}


/*
int pmm_init(size_t phy_mm_size, unsigned int k_start, size_t k_size, unsigned int phy_loc, unsigned int bmap_start) {
	int ret = 0;
	int oh_pg_cnt 	= 0;
	int k_pg_cnt	= 0;
	
	mm_size 		= phy_mm_size;
	bitmap_start	= bmap_start;
	mapped_pg_cnt	= phy_mm_size * PG_CNT_MB;
	bitmap_size		= mapped_pg_cnt >> DIV_BITS;		// or, pg_cnt / 8 
	oh_pg_cnt		= pmm_pg_cnt(phy_mm_size);
	k_pg_cnt		= pmm_pg_cnt(k_size);
	
	// make sure we have enough space for our bitmap 
	if ((bitmap_start + bitmap_size) <= 0xFFFFFFFF) {
		// make sure bitmap is zero'd, marked as unused
		memset((void *)bitmap_start, 0, bitmap_size);
	
		// mark any regions used by the kernel
		mark_used_cont(phy_loc, oh_pg_cnt);
		mark_used_cont(k_start, k_pg_cnt);
	} else {
		ret = PMM_ERR_BMAP_OOM;
	}
	
	return ret;
}
*/

static void mark_allocated_cont(addr_t addr, int cnt) {
	for (int i = 0; i < cnt; i++) {
		mark_allocated(addr + (i * 0x1000));
	}
}

static unsigned char *get_address(addr_t pg_addr) {
	return (unsigned char *)bm_region.start + BM_OFFSET(pg_addr);
}

/*
static void mark_unused(unsigned int pg_addr) {
	*(get_address(pg_addr)) &= ~(PG_USED << BIT_OFFSET(pg_addr));
}
*/

static void mark_allocated(addr_t pg_addr) {
	*(get_address(pg_addr)) |= (PG_USED << BIT_OFFSET(pg_addr));
}

bool is_page_allocated(addr_t pg_addr) {
	return *(get_address(pg_addr)) & (PG_USED << BIT_OFFSET(pg_addr));
}

