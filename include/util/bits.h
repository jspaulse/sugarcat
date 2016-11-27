#ifndef BITS_H
#define BITS_H
#include <stdbool.h>
/**
 * idx_lsb
 * Index Least Significant Bit
 * 
 * returns the index+1 of the list significant bit or 0 if x == 0
 * 
 * @x	unsigned integer
 * @return index+1 of lsb or 0 if x == 0
 **/
inline int idx_lsb(unsigned int x) {
    return __builtin_ffs(x);
}


/**
 * idx_msb
 * Index Most Significant Bit
 * 
 * returns the index+1 of the most significant bit or 0 if x == 0
 * @x	unsigned integer
 * @return index+1 of msb or 0 if x == 0
 **/
inline int idx_msb(unsigned int x) {
    int ret = 0;
	
    if (x > 0) {
	ret = ((sizeof(x) << 3) - __builtin_clz(x));
    }
	
    return ret;
}

/**
 * clr_lv_msb
 * Clear Leave Most Significant Bit
 * 
 * returns only the most significant bit
 * @x	unsigned integer
 * @return most significant bit
 **/
inline unsigned int clr_lv_msb(unsigned int x) {
    unsigned int ret = 0;
	
    if (x > 0) {
	ret = (1 << (idx_msb(x) - 1));
    }
    
    return ret;
}

/**
 * clr_lv_lsb
 * Clear Leave Least Significant Bit
 * 
 * returns only the least significant bit
 * @x	unsigned integer
 * @return least significant bit
 **/
inline unsigned int clr_lv_lsb(unsigned int x) {
    unsigned int ret = 0;
	
    if (x > 0) {
	ret = (1 << (idx_lsb(x) - 1));
    }
	
    return ret;
}

inline bool is_aligned_n(addr_t addr, int n) {
    return ((addr & (n - 1)) == 0);
}

inline unsigned int lowest_bit(unsigned int x) {
    return (x & (int)-x);
}

inline unsigned int highest_bit(unsigned int x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
	
    return x - (x >> 1);
}

inline bool is_power_of_two(unsigned int x) {
    return ((x != 0) && !(x & (x - 1)));
}

inline int loc_lowest_bit(unsigned int x) {
    return __builtin_ffs(x) - 1;
}

#endif
