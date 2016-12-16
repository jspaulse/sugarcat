#ifndef MM_H
#define MM_H
#include <stddef.h>
#include <stdbool.h>
#include <types.h>

/**
 * mm_reg
 * 
 * represents a region of memory
 * 
 * @base	base address of memory
 * @size	size of memory region
 **/
struct mm_reg {
    addr_t 	base;
    size_t	size;
};

/**
 * mm_vreg
 * 
 * represents a region with a virtual & physical
 * address.
 * 
 * @phy_base	physical base address
 * @virt_base	virtual base address
 * @size	size of region
 **/
struct mm_vreg {
    addr_t	phy_base;
    addr_t	virt_base;
    size_t	size;
};

/**
 * struct mm_resv_reg
 * 
 * represents a reserved region of memory.
 * 
 * @vbase	virtual address of base region
 * @phy		physical region
 **/
struct mm_resv_reg {
    addr_t		vbase;
    struct mm_reg	phy;
};

/**
 * is_within_region
 * 
 * determines if a sub region is within a region
 * 
 * @region	main region
 * @sub_region	sub region
 * @return true if sub region is within main region
 **/
inline bool is_within_region(struct mm_reg *region, struct mm_reg *sub_region) {
    bool ret = false;
	
    if (region != NULL && sub_region != NULL) {
	addr_t sub_end 	= sub_region->base + sub_region->size;
	addr_t main_end	= region->base + region->size;
		
	if (sub_region->base >= region->base) {
	    if (sub_end <= main_end) {
		ret = true;
	    }
	}
    }
	
    return ret;
}

/**
 * is_overlapping
 * 
 * determines if a region is overlapping another region
 * 
 * @region		main region (addressed lower)
 * @sub_region	sub region (addressed higher)
 * 
 * @return true if overlapping
 **/
inline bool is_overlapping(struct mm_reg *region, struct mm_reg *sub_region) {
    bool ret = false;
	
    if (region != NULL && sub_region != NULL) {
	addr_t main_end = region->base + region->size;
	addr_t sub_end	= sub_region->base + sub_region->size;
		
	if (is_within_region(region, sub_region)) {
	    if ((sub_end > region->base) && (sub_region->base < main_end)) {
		ret = true;
	    }
	}
    }
	
    return ret;
}

#endif
