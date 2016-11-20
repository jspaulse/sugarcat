#ifndef ARMV7_MMU_H
#define ARMV7_MMU_H
#include <arch/arm/armv7/armv7_syscntl.h>
#include <mm/mem.h>
#include <errno.h>
#include <stdbool.h>

/* pgd entries */
#define ARMV7_L1_INVALID	0x0
#define ARMV7_L1_PG_TB		0x1
#define ARMV7_L1_SECT		0x2
#define ARMV7_L1_SUPER_SECT	0x3
#define ARMV7_L1_SHIFT_DOMAIN	5
#define ARMV7_L1_SECT_SHIFT_AP	10
/* pgt entries */
#define ARMV7_L2_INVALID	0x0
#define ARMV7_L2_LARGE_PG	0x1
#define ARMV7_L2_SMALL_PG	0x2
#define ARMV7_L2_SHIFT_AP	4
/* simplified access */
#define ARMV7_AP_KRW_UNO	0x0
#define ARMV7_AP_KRW_URW	0x1
#define ARMV7_AP_KRO_UNO	0x2
#define ARMV7_AP_KRO_URO	0x3
/* pgd */
#define KERN_PGD		0x1
#define USER_PGD		0x2
#define PGD_SZ			0x2000
#define PGD_ENTRY_CNT		2048
#define PGD_SECT_MASK		0xFFF00000
#define PGD_TBL_MASK		0xFFFFFC00
/* pg_tb */
#define PG_TB_SZ		0x200000
#define PG_TB_ENTRY_SZ		0x400
/* dacr */
#define DACR_DOMAIN_CNT		15
#define DACR_MASK		0x3
/* ttbr0 */
#define TTBR_MASK		0xFFFFC000

/**
 * armv7_is_supported_pgd_type
 * 
 * determins if a page dir entry type is supported
 * @type	type to check
 * @return true if supported
 **/
inline bool armv7_is_supported_pgd_type(unsigned char type) {
    bool ret = false;
    
    switch (type) {
	case ARMV7_L1_INVALID:
	case ARMV7_L1_PG_TB:
	case ARMV7_L1_SECT:
	    ret = true;
	    break;
    }
    
    return ret;
}

/**
 * armv7_is_supported_pgt_type
 * 
 * determines if a page table entry type is supported
 * @type	type to check
 * @return true if supported
 **/
inline bool armv7_is_supported_pgt_type(unsigned char type) {
    bool ret = false;
    
    switch(type) {
	case ARMV7_L2_INVALID:
	case ARMV7_L2_LARGE_PG:
	    ret = true;
	    break;
    }
    
    return ret;
}


/**
 * armv7_set_domain
 * 
 * sets the access permissions of a specified domain to specified flag
 * 
 * @domain	domain to set permissions
 * @acc_perm	permissions of domain
 * 
 * note: domain must be in valid range (0-15)
 * @return errno
 **/
inline int armv7_set_domain(unsigned char domain, unsigned char acc_perm) {
    unsigned int reg 	= armv7_get_dacr();
    int shift		= domain * 2;
    int ret 		= ERR_SUCC;
	
    if (domain <= DACR_DOMAIN_CNT) {
	reg &= ~(DACR_MASK << shift);
	reg |= ((acc_perm & DACR_MASK) << shift);
		
	/* write back */
	armv7_set_dacr(reg);
    } else {
	ret = ERR_INVAL;
    }
	
    return ret;
}

/**
 * armv7_is_mmu_enabled
 * 
 * determines if the mmu is enabled
 * 
 * @return true if enabled
 **/
inline bool armv7_is_mmu_enabled(void) {
    return (armv7_get_sctlr() & ARMV7_SCTLR_MMU_ENB);
}

/**
 * armv7_create_2g_pgd_to_pgtb
 * 
 * creates a map between a page directory and subsequent page tables
 * for a 2g:2g split. all entries created in page tables and invalid
 * 
 * this assumes that the regions pgd and pg_tb are continuous and that
 * page directory entries are every 1024 bytes.
 * 
 * this also assumes a uniform domain (and !xn) for every entry.
 * 
 * @pgd		page directory
 * @pg_tb	page tables
 * @domain	domain flags
 * @return errno
 **/
inline int armv7_create_2g_pgd_to_pgtb(addr_t pgd, addr_t pg_tb, unsigned char domain) {
    addr_t *pg_dir	= (addr_t *)pgd;
    int ret 		= ERR_SUCC;
    
    /* clean (invalidate) all entries */
    memset(pg_dir, 0, PG_TB_SZ);
    
    /* 
     * PGD_ENTRY_CNT number of entries,
     * iterate through them all and write
     * their associated pg_tb address w/domain
     */
    for (int i = 0; i < PGD_ENTRY_CNT; i++) {
	addr_t pg_tb_entry = (pg_tb + (i * PG_TB_ENTRY_SZ));
	
	pg_dir[i] = (pg_tb_entry | (domain << 5) | ARMV7_L1_PG_TB);
    }
    
    return ret;
}

/* armv7_mmu.c */
int armv7_mmu_init_prior_enable(void);
int armv7_mmu_set_pgd(addr_t address, unsigned char flags, unsigned char pg_dir);
int armv7_mmu_map_pgd(addr_t paddr, addr_t vaddr, unsigned char domain, unsigned char acc_perm, unsigned char type, unsigned char flags, bool update);

#endif
