#ifndef PMM_H
#define PMM_H
#include <util/bits.h>
#include <types.h>
#include <stddef.h>

#define PMM_ERR_NOT_ENOUGH_MEM	-1

#define BIT_MASK		0x7000
#define PG_MASK			0xFFFFF000
#define DIV_PG			12
#define DIV_MB			20
#define DIV_BITS		3
//#define PG_CNT_MB		256
#define PG_USED			1
#define PG_UNUSED		0
#define PG_SZ			4096 /* 0x1000 */

struct mm_region {
	addr_t		start;
	size_t		size;
};


/**
 * mem_pg_cnt
 * 
 * returns the page count for the size mem_sz
 * 
 * @mem_sz	size of memory region
 * 
 * @return page count
 **/
inline unsigned int mem_pg_cnt(size_t mem_sz) {
	unsigned int ret = (mem_sz >> DIV_PG);
	
	if (mem_sz & PG_MASK) {
		ret++;
	}
	
	return ret;
}

/**
 * bitmap_size
 * 
 * returns the overhead size (in bytes) for a bitmap
 * representing a memory region sized mem_sz
 * 
 * @mem_sz	memory region size
 * 
 * @return overhead cost in bytes
 **/
inline unsigned int bitmap_size(size_t mem_sz) {
	return (mem_sz >> DIV_PG) >> DIV_BITS;
}

/**
 * bitmap_oh_pg_cnt
 * 
 * returns the overhead page count for a bitmap representing 
 * a memory region sized mem_sz
 * 
 * @mem_sz	memory region size
 * 
 * @return overhead cost in pages
 **/
inline unsigned int bitmap_oh_pg_cnt(size_t mem_sz) {
	unsigned int ret = bitmap_size(mem_sz);
	
	if (!is_power_of_two(ret)) {
		ret = highest_bit(ret) << 1;
	}
	
	return ret >> DIV_PG;
}
int pmm_init_test(struct mm_region phy_mem, addr_t bm_phy_start, addr_t bm_vir_start, struct mm_region *used, int used_cnt);
bool is_page_allocated(addr_t pg_addr);
#endif
