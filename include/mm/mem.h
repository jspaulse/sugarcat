#ifndef MEM_H
#define MEM_H
#include <util/bits.h>
#include <stddef.h>
#include <types.h>

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
inline unsigned int memr(addr_t address) {
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
inline void memw(addr_t address, unsigned int data) {
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

/**
 * memcpy
 * 
 * memory copy
 * 
 * copies a region of memory of specified size from specified region
 * into specified destination. specified.
 * 
 * @dest	pointer to destination
 * @src		pointer to source
 * @size	size (in bytes) to copy
 **/
inline void memcpy(void *dest, const void *src, size_t size) {
    unsigned char *dptr = (unsigned char *)dest;
    unsigned char *sptr = (unsigned char *)src;
	
    for (unsigned int i = 0; i < size; i++) {
	dptr[i] = sptr[i];
    }
}

extern void mach_init_printf(const char *, ...);
/**
 * memconvle32
 * 
 * memory convert to little endian 32
 * converts a region of big-endian memory to little endian
 * @src		pointer to region
 * @size	size (in bytes) of region
 */
inline void memconvle32(void *src, size_t size) {
    uint32_t 	*ptr 	= (uint32_t *)src;
    size_t 	act	= size >> 2;	/* size / 4 */
    
    for (unsigned int i = 0; i < act; i++) {
	ptr[i] = be32_to_cpu(ptr[i]);
    }
}

/**
 * memconvle64
 * 
 * memory convert to little endian 64
 * converts a region of big-endian memory to little endian
 * @src		pointer to region
 * @size	size (in bytes) of region
 **/
inline void memconvle64(void *src, size_t size) {
    uint64_t	*ptr	= (uint64_t *)src;
    size_t	act	= size >> 3;	/* size /  8 */
    
    for (unsigned int i = 0; i < act; i++) {
	ptr[i] = be64_to_cpu(ptr[i]);
    }
}


#endif
