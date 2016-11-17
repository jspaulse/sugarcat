#ifndef ARMV7_MMU_H
#define ARMV7_MMU_H
#include <arch/arm/armv7/armv7_syscntl.h>
#include <errno.h>

#define ARMV7_L1_INVALID	0x0
#define ARMV7_L1_PG_TB		0x1
#define ARMV7_L1_SECT		0x2
#define ARMV7_L1_SUPER_SECT	0x3

#define ARMV7_L2_INVALID	0x0
#define ARMV7_L2_LARGE_PG	0x1
#define ARMV7_L2_SMALL_PG	0x2

/* Simplified Access */
#define ARMV7_AP_KRW_UNO	0x0
#define ARMV7_AP_KRW_URW	0x1
#define ARMV7_AP_KRO_UNO	0x2
#define ARMV7_AP_KRO_URO	0x3

/* dacr */
#define DACR_DOMAIN_CNT		15
#define DACR_MASK			0x3


/**
 * armv7_set_domain
 * 
 * sets the access permissions of a specified domain to specified flag
 * 
 * @domain		domain to set permissions
 * @acc_perm	permissions of domain
 * 
 * note: domain must be in valid range (0-15)
 * @return errno
 **/
inline int armv7_set_domain(unsigned char domain, unsigned char acc_perm) {
	unsigned int reg 	= armv7_get_dacr();
	int shift			= domain * 2;
	int ret 			= ERR_SUCC;
	
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


#endif
