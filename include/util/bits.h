#ifndef BITS_H
#define BITS_H
#include <stdbool.h>

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
