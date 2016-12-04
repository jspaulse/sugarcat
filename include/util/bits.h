#ifndef BITS_H
#define BITS_H
#include <stdint.h>
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

/**
 * is_aligned_n
 * 
 * determines if an address is aligned to n alignment
 * 
 * @addr	address to determine alignment
 * @n		alignment; i.e.,
 * 		if checking for 16KiB alignment, n == 0x4000 (16384)
 * @return true if aligned n
 **/
inline bool is_aligned_n(addr_t addr, unsigned int n) {
    return ((addr & (n - 1)) == 0);
}

/**
 * is_power_of_two
 * 
 * determines if x is power of two
 * @x	x
 * @return true if power of two
 **/
inline bool is_power_of_two(unsigned int x) {
    return ((x != 0) && !(x & (x - 1)));
}

/**
 * be16_to_le16
 * 
 * converts a big-endian 16 bit unsigned integer to little-endian
 * @x	unsigned integer to convert
 * @return little-endian integer
 **/
inline uint16_t be16_to_le16(uint16_t x) {
    return ((x >> 8) | (x << 8));
}

/**
 * be32_to_le32
 * 
 * converts a big-endian 32 bit unsigned integer to little-endian
 * @x	unsigned integer to convert
 * @return little-endian integer
 **/
inline uint32_t be32_to_le32(uint32_t x) {
    uint32_t ret = (((x & 0xFF) << 24) 	| 	/* 0 > 3 */
	((x & 0xFF00) << 8) 		| 	/* 1 > 2 */
	((x & 0xFF0000) >> 8) 		| 	/* 2 > 1 */
	((x & 0xFF000000) >> 24)); 		/* 3 > 0 */
    
    return ret;
}

/**
 * be64_to_le64
 * 
 * converts a big-endian 64 bit unsigned integer to little-endian
 * @x	unsigned integer to convert
 * @return little-endian integer
 **/
inline uint64_t be64_to_le64(uint64_t x) {
    uint64_t ret = (((x & 0xFF) << 56) 	|	/* 0 > 7 */
	((x & 0xFF00) << 40)		|	/* 1 > 6 */
	((x & 0xFF0000) << 24)		|	/* 2 > 5 */
	((x & 0xFF000000) << 8)		|	/* 3 > 4 */
	((x & 0xFF00000000) >> 8)	|	/* 4 > 3 */
	((x & 0xFF0000000000) >> 24)	|	/* 5 > 2 */
	((x & 0xFF000000000000) >> 40)	|	/* 6 > 1 */
	((x >> 56) & 0xFF));			/* 7 > 0 */
    
    return ret;
}

#endif
