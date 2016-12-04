#ifndef ARMV7_MMU_H
#define ARMV7_MMU_H
#include <arch/arm/armv7/armv7_syscntl.h>
#include <mm/mem.h>
#include <errno.h>
#include <stdbool.h>

#define PG_DIV_MASK		0x7
#define PGD_ENTRY_CNT		4096
#define PGD_IDX_SHIFT		20
#define PGD_SECT_AP_SHIFT	10
#define PGD_DOMAIN_SHIFT	5
#define PGD_DOMAIN_MASK		(0xF << PGD_DOMAIN_SHIFT)
#define PGD_AP_MASK		(0x3 << PGD_SECT_AP_SHIFT)
#define PGD_TYPE_MASK		0x3
#define PGD_SECT_MASK		0xFFF00000
#define PGD_TABLE_MASK		0xFFFFFC00

#define PGTB_SZ			0x400
#define PGTB_AP_SHIFT		4
#define PGTB_IDX_SHIFT		12
#define PGTB_IDX_MASK		0xFF
#define PGTB_LG_PG_MASK		0xFFFF0000
#define PGTB_SM_PG_MASK		0xFFFFF000
#define PGTB_TYPE_MASK		0x3
/* domains */
#define USER_DOMAIN		0
#define KERN_DOMAIN		1
/* dacr */
#define DACR_DOMAIN_CNT		15
#define DACR_MASK		0x3
/* ttbcr */
#define TTBCR_N_MASK		0x7
/* ttbr */
#define TTBR_ALIGN 		14
#define TTBR_MASK		0xFFFFC000

/**
 * armv7_mmu_pgd_type
 * 
 * defines types of mmu page directory entries
 **/
typedef enum {
    ARMV7_MMU_PGD_INVALID	= 0x0,
    ARMV7_MMU_PGD_TABLE		= 0x1,
    ARMV7_MMU_PGD_SECTION	= 0x2,
    ARMV7_MMU_PGD_SUPER_SECTION	= 0x3
} armv7_mmu_pgd_type;

/**
 * armv7_mmu_pgtb_type
 * 
 * defines types of mmu page table entries
 **/
typedef enum {
    ARMV7_MMU_PGTB_INVALID	= 0x0,
    ARMV7_MMU_PGTB_LARGE_PG	= 0x1,
    ARMV7_MMU_PGTB_SMALL_PG	= 0x2
} armv7_mmu_pgtb_type;

/**
 * armv7_mmu_acc_perm
 * 
 * defines access permissions for mmu entries
 **/
typedef enum {
    ARMV7_MMU_ACC_KRW_NOU	= 0x0,
    ARMV7_MMU_ACC_KRW_URW	= 0x1,
    ARMV7_MMU_ACC_KRO_NOU	= 0x2,
    ARMV7_MMU_ACC_KRO_URO	= 0x3
} armv7_mmu_acc_perm;

/**
 * armv7_mmu_pgd_entry
 * 
 * defines a mmu entry for page directories
 * 
 * @phy_addr	physical address of entry
 * @virt_addr	virtual address of entry
 * @domain	domain access of entry
 * @acc_perm	access permission of entry
 * @type	type of entry
 * @flags	additional entry flags
 **/
struct armv7_mmu_pgd_entry {
    addr_t		phy_addr;
    addr_t		virt_addr;
    unsigned char	domain;
    armv7_mmu_acc_perm	acc_perm;
    armv7_mmu_pgd_type	type;
    unsigned int	flags;
};

/**
 * armv7_mmu_pgtb_entry
 * 
 * defines a mmu entry for page tables
 * 
 * @phy_addr	physical address of entry
 * @virt_addr	virtual address of entry
 * @acc_perm	access permission of entry
 * @type	type of entry
 * @flags	additional entry flags
 **/
struct armv7_mmu_pgtb_entry {
    addr_t		phy_addr;
    addr_t		virt_addr;
    armv7_mmu_acc_perm	acc_perm;
    armv7_mmu_pgtb_type	type;
    unsigned int	flags;
};

/**
 * armv7_is_supported_pgd_type
 * 
 * determines if a page directory entry type is supported
 * 
 * @type	type to check
 * @return true if supported
 **/
inline bool armv7_is_supported_pgd_type(armv7_mmu_pgd_type type) {
    bool ret = false;
    
    switch (type) {
	case ARMV7_MMU_PGD_INVALID:
	case ARMV7_MMU_PGD_SECTION:
	case ARMV7_MMU_PGD_TABLE:
	    ret = true;
	    break;
	case ARMV7_MMU_PGD_SUPER_SECTION:
	    ret = false;
	    break;
    }
    
    return ret;
}

/**
 * armv7_is_supported_pgtb_type
 * 
 * determines if a page table entry type is supported
 * 
 * @type	type to check
 * @return true if supported
 **/

inline bool armv7_is_supported_pgtb_type(armv7_mmu_pgtb_type type) {
    bool ret = false;
    
    switch (type) {
	case ARMV7_MMU_PGTB_INVALID:
	case ARMV7_MMU_PGTB_SMALL_PG:
	    ret = true;
	    break;
	case ARMV7_MMU_PGTB_LARGE_PG:
	    ret = false;
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
    int ret 		= ESUCC;
	
    if (domain <= DACR_DOMAIN_CNT) {
	reg &= ~(DACR_MASK << shift);
	reg |= ((acc_perm & DACR_MASK) << shift);
		
	/* write back */
	armv7_set_dacr(reg);
    } else {
	ret = EINVAL;
    }
	
    return ret;
}

/**
 * armv7_mmu_is_enabled
 * 
 * determines if the mmu is enabled
 * 
 * @return true if enabled
 **/
inline bool armv7_mmu_is_enabled(void) {
    return (armv7_get_sctlr() & ARMV7_SCTLR_MMU_ENB);
}

/**
 * armv7_mmu_get_pg_div
 * 
 * returns the mmu page div
 * @return pg_div
 **/
inline int armv7_mmu_get_pg_div(void) {
    return (armv7_get_ttbcr() & TTBCR_N_MASK);
}


/* armv7_mmu.c */
addr_t armv7_mmu_get_user_pgd(void);
addr_t armv7_mmu_get_kern_pgd(void);
int armv7_mmu_set_kern_pgd(addr_t pgd_addr, unsigned char flags);
int armv7_mmu_set_user_pgd(addr_t pgd_addr, unsigned char flags);

int armv7_mmu_map_pgd(struct armv7_mmu_pgd_entry *pgd_ent);
int armv7_mmu_map_pgtb(struct armv7_mmu_pgtb_entry *pgtb_ent);

int armv7_mmu_map_new_pgd(addr_t pgd_addr, struct armv7_mmu_pgd_entry *pgd_ent);
int armv7_mmu_map_new_pgtb(addr_t pgtb_addr, struct armv7_mmu_pgtb_entry *pgtb_ent);

addr_t armv7_mmu_virt_to_phy(addr_t virt_addr);


#endif
