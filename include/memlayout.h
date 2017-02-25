#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H
#include <mm/mm.h>
#include <stddef.h>
#include <types.h>

/* kernel.ld */
extern addr_t kp_start;		/* kernel physical start	*/
extern addr_t kv_start; 	/* kernel virtual start		*/
extern addr_t k_stack;		/* kernel stack			*/
extern addr_t k_pgd;		/* kernel page directory	*/
extern addr_t lmi_start;	/* low memory init. start	*/
extern addr_t lmi_end;		/* low memory init. end		*/
extern addr_t lmi_bss_start;	/* lmi bss start		*/
extern addr_t lmi_bss_end;	/* lmi bss end			*/
extern addr_t hmi_start;	/* high memory init. start	*/
extern addr_t hmi_end;		/* high memory init. end	*/
extern addr_t hmi_bss_start;	/* hmi bss start		*/
extern addr_t hmi_bss_end;	/* hmi bss end			*/
extern addr_t k_start;		/* kernel start			*/
extern addr_t k_end;		/* kernel end			*/
extern addr_t k_bss_start;	/* kernel bss start		*/
extern addr_t k_bss_end;	/* kernel bss end		*/		

/**
 * mlay_get_kern_phy_start
 * Memory_Layout Get Kernel Physical Start
 * 
 * returns the physical start address of the kernel
 * NOTE: this includes the low/high init. regions of the kernel
 * @return starting physical address of kernel
 **/
inline addr_t mlay_get_kern_phy_start() {
    return (addr_t)&kp_start;
}

/**
 * mlay_get_kern_virt_start
 * Memory_Layout Get Kernel Virtual Start
 * 
 * returns the virtual start address of the kernel
 * @return starting virtual address of kernel
 **/
inline addr_t mlay_get_kern_virt_start() {
    return (addr_t)&kv_start;
}

/**
 * mlay_get_kern_stack
 * 
 * returns the physical address of the kernel stack
 * NOTE: this may only be useful for initialization and 
 * may not be applicable at later stages.
 * @return kernel stack
 **/
inline addr_t mlay_get_kern_stack() {
    return (addr_t)&k_stack;
}

/**
 * mlay_get_kern_pgd
 * 
 * returns the physical address of the kernel page directory
 * @return kernel page directory
 **/
inline addr_t mlay_get_kern_pgd() {
    return (addr_t)&k_pgd;
}

/**
 * mlay_get_lmi_start
 * 
 * returns the physical start address of the low memory init. region
 * of the kernel
 * 
 * @return physical starting address of low mem. init. region
 **/
inline addr_t mlay_get_lmi_start() {
    return (addr_t)&lmi_start;
}

/**
 * mlay_get_lmi_region_sz
 * 
 * returns the size (in bytes) of the low memory init. region
 * @return size (in bytes) of low memory init. region
 **/
inline size_t mlay_get_lmi_region_sz() {
    return (size_t)((addr_t)&lmi_end - (addr_t)&lmi_start);
}

/**
 * mlay_get_lmi_bss_start
 * 
 * returns the physical start address of the low memory init. bss
 * @return physical starting address of low memory init. bss
 **/
inline addr_t mlay_get_lmi_bss_start() {
    return (addr_t)&lmi_bss_start;
}

/**
 * mlay_get_lmi_bss_region_sz
 * 
 * returns the size (in bytes) of the low memory init. bss region
 * @return size (in bytes) of low memory init. bss region
 **/
inline size_t mlay_get_lmi_bss_region_sz() {
    return (size_t)((addr_t)&lmi_bss_end - (addr_t)&lmi_bss_start);
}

/**
 * mlay_get_hmi_start
 * 
 * returns the virtual starting address of the high memory init. region
 * @return virtual starting address of high memory init.
 **/
inline addr_t mlay_get_hmi_start() {
    return (addr_t)&hmi_start;
}

/**
 * mlay_get_hmi_region_sz
 * 
 * returns the size (in bytes) of the high memory init. region
 * @return size (in bytes) of the hmi region
 **/
inline size_t mlay_get_hmi_region_sz() {
    return (size_t)((addr_t)&hmi_end - (addr_t)&hmi_start);
}

/**
 * mlay_get_hmi_bss_start
 * 
 * returns the virtual starting address of the hmi bss region
 * @return virtual starting address of hmi bss region
 **/
inline addr_t mlay_get_hmi_bss_start() {
    return (addr_t)&hmi_bss_start;
}

/**
 * mlay_get_hmi_bss_region_sz
 * 
 * returns the size (in bytes) of the hmi bss region
 * @return size (in bytes) of hmi region size
 **/
inline size_t mlay_get_hmi_bss_region_sz() {
    return (size_t)((addr_t)&hmi_bss_end - (addr_t)&hmi_bss_start);
}

/**
 * mlay_get_kern_start
 * 
 * returns the virtual starting address of the kernel region
 * @return virtual starting address of kernel region
 **/
inline addr_t mlay_get_kern_start() {
    return (addr_t)&k_start;
}

/**
 * mlay_get_kern_region_sz
 * 
 * returns the size (in bytes) of the kernel region
 * @return size (in bytes) of kernel region
 **/
inline size_t mlay_get_kern_region_sz() {
    return (size_t)((addr_t)&k_end - (addr_t)&k_start);
}

/**
 * mlay_get_kern_bss_start
 * 
 * returns the virtual starting address of the kernel bss region
 * @return virtual starting address of kernel bss region
 **/
inline addr_t mlay_get_kern_bss_start() {
    return (addr_t)&k_bss_start;
}

/**
 * mlay_get_kern_bss_region_sz
 * 
 * returns the size (in bytes) of the kernel bss region
 * @return size (in bytes) of kernel bss region
 **/
inline size_t mlay_get_kern_bss_region_sz() {
    return (size_t)((addr_t)&k_bss_end - (addr_t)&k_bss_start);
}

/**
 * kvm_to_phy
 * Kernel Virtual Memory to Physical
 * 
 * converts a kernel virtual address to it's corresponding physical
 * address.
 * NOTE:  this is only useful in the context of initialization.
 * 
 * @address	virtual address to convert
 * @return	physical address
 **/
inline addr_t kvm_to_phy(addr_t address) {
    return ((address & ~mlay_get_kern_virt_start()) |
	mlay_get_kern_phy_start());
}

/**
 * phy_to_kvm
 * Physical to Kernel Virtual Memory
 * 
 * converts a kernel physical address to it's corresponding virtual 
 * address.
 * NOTE:  this is only useful in the context of initialization.
 * 
 * @address	physical address to convert
 * @return	virtual address
 **/
inline addr_t phy_to_kvm(addr_t address) {
    return ((address & ~mlay_get_kern_phy_start()) |
	mlay_get_kern_virt_start());
}

/* memlayout.c */
int mlay_get_phy_mem_reg(addr_t fdt_base, struct mm_reg *mem_reg);
int mlay_get_initrd_reg(addr_t fdt_base, struct mm_reg *initrd_reg);

#endif
