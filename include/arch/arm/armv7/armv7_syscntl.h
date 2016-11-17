#ifndef ARMV7_SYSCNTL_H
#define ARMV7_SYSCNTL_H
#include <types.h>

/* sctlr */
#define ARMV7_SCTLR_MMU_ENB 0x1
#define ARMV7_SCTLR_AFE		0x20000000
/* dacr */
#define ARMV7_DACR_NO_ACC	0x0
#define ARMV7_DACR_CLIENT	0x1
#define ARMV7_DACR_RESVD	0x2
#define ARMV7_DACR_MNGR		0x3
/* ttbcr */
#define ARMV7_TTBCR_2G_2G	0x1

/**
 * armv7_get_config_base
 * 
 * returns the Configuration Base Address
 * 
 * @return config base addr
 **/
inline unsigned int armv7_get_config_base(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 4, %0, c15, c0, 0" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_get_ivt_base
 * 
 * returns the Interrupt Vector Table base address
 * 
 * @return ivt base addr
 **/
inline unsigned int armv7_get_ivt_base(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c12, c0, 0" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_ivt_base
 * 
 * sets the Interrupt Vector Table base address
 * 
 * @address	base address for vector table
 **/
inline void armv7_set_ivt_base(addr_t address) {
	asm volatile("mcr p15, 0, %0, c12, c0, 0" : : "r" (address));
}

/**
 * armv7_get_sctlr
 * 
 * returns the current System Control Register
 * 
 * @return System Control Register
 **/
inline unsigned int armv7_get_sctlr(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_sctlr
 * 
 * sets the System Control Register to specified value
 * 
 * @val specified value
 **/
inline void armv7_set_sctlr(unsigned int val) {
	asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (val));
}

/**
 * armv7_get_aux_cntl
 * 
 * returns the current Auxiliary Control Register
 *
 * @return Auxiliary Control Register
 **/
inline unsigned int armv7_get_aux_cntl(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c1, c0, 1" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_aux_cntl
 * 
 * sets the Auxilary Control Register to specified value
 * 
 * @val specified value
 **/
inline void armv7_set_aux_cntl(unsigned int val) {
	asm volatile("mcr p15, 0, %0, c1, c0, 1" : : "r" (val));
}

/**
 * armv7_get_scr
 * 
 * returns the current Secure Configuration Register
 * @return Secure Configuration Register
 **/
inline unsigned int armv7_get_scr(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c1, c1, 0" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_scr
 * 
 * sets the Secure Configuration Register to specified value
 * @val specified value
 **/
inline void armv7_set_scr(unsigned int val) {
	asm volatile("mcr p15, 0, %0, c1, c1, 0" : : "r" (val));
}

/**
 * armv7_get_ttbcr
 * 
 * returns the current Translation Table Base Control Register
 * @return Translation Table Base Control Register
 **/
inline unsigned int armv7_get_ttbcr(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c2, c0, 2" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_ttbcr
 * 
 * sets the Translation Table Base Control Register to specified value
 * @val	specified value
 **/
inline void armv7_set_ttbcr(unsigned int val) {
	asm volatile("mcr p15, 0, %0, c2, c0, 2" : : "r" (val));
}

/**
 * armv7_get_ttbr0
 * 
 * returns the current Translation Table Base Register #0
 * @return Translation Table Base Register #0
 **/
inline unsigned int armv7_get_ttbr0(void) {
	unsigned int ret = 0;
	
	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_ttrb0
 * 
 * sets the Translation Table Base Register #0 to specified value
 * @val	specified value
 **/
inline void armv7_set_ttbr0(unsigned int val) {
	asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r" (val));
}

/**
 * armv7_get_ttbr1
 * 
 * returns the current Translation Table Base Register #1
 * @return Translation Table Base Register #1
 **/
inline unsigned int armv7_get_ttbr1(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c2, c0, 1" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_ttbr1
 * 
 * sets the Translation Table Base Register #1 to specified value
 * @val	specified value
 **/
inline void armv7_set_ttbr1(unsigned int val) {
	asm volatile("mcr p15, 0, %0, c2, c0, 1" : : "r" (val));
}

/**
 * armv7_get_dacr
 * 
 * returns the current Domain Access Control Register
 * @return Domain Access Control Register
 **/
inline unsigned int armv7_get_dacr(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c3, c0, 0" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_set_dacr
 * 
 * sets the Domain Access Control Register to specified value
 * @val	specified value
 **/
inline void armv7_set_dacr(unsigned int val) {
	asm volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (val));
}

/**
 * armv7_get_mpidr
 * 
 * returns the current Multiprocessor Affinity Register
 * @return Multiprocessor Affinity Register
 **/
inline unsigned int armv7_get_mpidr(void) {
	unsigned int ret = 0;
	
	asm volatile("mrc p15, 0, %0, c0, c0, 5" : "=r" (ret));
	
	return ret;
}

/**
 * armv7_invalidate_unified_tlb
 * 
 * invalidates the unified tlb
 **/
inline void armv7_invalidate_unified_tlb(void) {
	asm volatile("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
}

#endif

