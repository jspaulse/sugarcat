#ifndef MEM_H
#define MEM_H
#include <stddef.h>
#include <arch/arch.h>

/**
 * memr
 * 
 * Peripheral Memory Read
 * 
 * read data from specified peripheral address
 * 
 * @address 	address to read data from
 * @return		data at specified address
 * 
 * NOTE:  This function should only be utilized when reading to a peripheral
 **/
inline unsigned int memr(unsigned int address) {
	return *(volatile unsigned int *)address;
}

/**
 * memw
 * 
 * Peripheral Memory Write
 * 
 * writes data to specified peripheral address
 * 
 * @address		address to write value to
 * @data		data to write
 * 
 * NOTE:  This function should only be utilized when writing to a peripheral
 **/
inline void memw(unsigned int address, unsigned int data) {
	*(volatile unsigned int *)address = data;
}

/**
 * memset
 * 
 * memory set
 * 
 * fills region of memory starting at dat and spanning to dat + size
 * with specified character
 * 
 * @dat		pointer to start of data region to be filled
 * @c		character to fill data region with
 * @size	number of bytes within region to fill
 **/
inline void memset(void *dat, unsigned char c, size_t size) {
	unsigned char *ptr = (unsigned char *)dat;

	for (unsigned int i = 0; i < size; i++) {
		ptr[i] = c;
	}
}

inline void memcpy(void *dest, const void *src, size_t size) {
	unsigned char *dptr = (unsigned char *)dest;
	unsigned char *sptr = (unsigned char *)src;
	
	for (unsigned int i = 0; i < size; i++) {
		dptr[i] = sptr[i];
	}
}



#endif
