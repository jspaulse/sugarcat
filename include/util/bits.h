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
 * determines if a value x is aligned to n alignment
 * 
 * @x		value
 * @n		alignment; i.e.,
 * 		if checking for 16KiB alignment, n == 0x4000 (16384)
 * @return true if aligned n
 **/
inline bool is_aligned_n(unsigned int x, unsigned int n) {
    return ((x & (n - 1)) == 0);
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
 * is_little_endian
 * 
 * determines if the current endianness is little-endian
 * @return true if little endian
 **/
inline bool is_little_endian(void) {
    bool 		ret 	= false;
    unsigned int	test	= 1;
    unsigned char	*ptr	= (unsigned char *)&test;
    
    if (*ptr == 1) {
	ret = true;
    }
    
    return ret;
}

/**
 * endswap16
 * 
 * endian swap 16bit
 * 
 * swaps a 16 bit unsigned integers endianness
 * @x	unsigned integer to swap
 * @return swapped integer
 **/
inline uint16_t endswap16(uint16_t x) {
    return ((x >> 8) | (x << 8));
}

/**
 * endswap32
 * 
 * endian swap 32bit
 * 
 * swaps a 32 bit unsigned integers endianness
 * @x	unsigned integer to swap
 * @return swapped integer
 **/
inline uint32_t endswap32(uint32_t x) {
    uint32_t ret = (((x & 0xFF) << 24) 	| 	
	((x & 0xFF00) << 8) 		| 	
	((x & 0xFF0000) >> 8) 		| 	
	((x & 0xFF000000) >> 24));
    
    return ret;
}

/**
 * endswap64
 * 
 * endian swap 64bit
 * 
 * swaps a 64 bit unsigned integers endianness
 * @x	unsigned integer to swap
 * @return swapped integer
 **/
inline uint64_t endswap64(uint64_t x) {
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

/**
 * cpu_to_be16
 * 
 * converts a cpu endianness unsigned integer to big endian
 * @x	integer to swap
 * @return big endian unsigned integer
 **/
inline uint16_t cpu_to_be16(uint16_t x) {
    uint16_t ret = x;
    
    if (is_little_endian()) {
	ret = endswap16(x);
    }
    
    return ret;
}

/**
 * be16_to_cpu
 * 
 * converts a big-endian 16 bit unsigned integer to cpu endianness
 * @x	integer to spawn
 * @return cpu endian unsigned integer
 **/
inline uint16_t be16_to_cpu(uint16_t x) {
    uint16_t ret = x;
    
    if (is_little_endian()) {
	ret = endswap16(x);
    }
    
    return ret;
}

/**
 * be32_to_cpu
 * 
 * converts a big-endian 32 bit unsigned integer to cpu endianness
 * @x	integer to spawn
 * @return cpu endian unsigned integer
 **/
inline uint32_t be32_to_cpu(uint32_t x) {
    uint32_t ret = x;
    
    if (is_little_endian()) {
	ret = endswap32(x);
    }
    
    return ret;
}

/**
 * be64_to_cpu
 * 
 * converts a big-endian 64 bit unsigned integer to cpu endianness
 * @x	integer to spawn
 * @return cpu endian unsigned integer
 **/
inline uint64_t be64_to_cpu(uint64_t x) {
    uint64_t ret = x;
    
    if (is_little_endian()) {
	ret = endswap64(x);
    }
    
    return ret;
}

#endif
