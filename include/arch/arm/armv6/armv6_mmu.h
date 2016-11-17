#ifndef ARMV6_MMU_H
#define ARMV6_MMU_H
#include <stdbool.h>
#include <mm/mmu.h>

#define SUBPAGE_AP_DISABLED		1 << 23
#define MMU_ENABLE				1
#define ARMV6_MMU_ACC_KRW		0x1 //APX and AP[1:0] = b001, Read-Write for Privileged code, No Access for Unprivileged
#define ARMV6_MMU_ACC_URW		0x3 //APX and AP[1:0] = b011, Read-Write for both Privileged and Unprivileged code
#define ARMV6_MMU_CACHED		1 << 3;
#define ARMV6_MMU_4KB			0x2
#define ARMV6_MMU_COARSE		0x1
#define ARMV6_MMU_SECTION		0x2
#define ARMV6_MMU_COARSE		0x1
#define ARMV6_MMU_2G_2G_SPLIT	1
#define ARMV6_MMU_1G_3G_SPLIT	2
#define ARMV6_NO_DOMAINS		0xFFFFFFFF

/*
void armv6_mmu_set_domain(unsigned int x);
void armv6_mmu_set_ttbcr(unsigned int x);
void armv6_mmu_set_ttb0(unsigned int x);
void armv6_mmu_set_ttb1(unsigned int x);

unsigned int armv6_read_cntl_reg(void);
void armv6_write_cntl_reg(unsigned int x);
void armv6_invalidate_tlbs(void);
void armv6_set_domain(unsigned int x);
*/
inline unsigned int armv6_pg_acc_to_arm(pg_acc_t pg_acc) {
	int ret = 0;
	
	switch(pg_acc) {
		case KERN_RW:
			ret = ARMV6_MMU_ACC_KRW;
			break;
		case USER_RW:
			ret = ARMV6_MMU_ACC_URW;
			break;
	}
	
	return ret;
}

inline unsigned int armv6_mmu_vm_map_to_arm(mmu_vm_map mvm) {
	int ret = 0;
	
	switch(mvm) {
		case MMU_2G_2G_SPLIT:
			ret = ARMV6_MMU_2G_2G_SPLIT;
			break;
		case MMU_1G_3G_SPLIT:
			ret = ARMV6_MMU_1G_3G_SPLIT;
		default:
			break;
	}
	
	return ret;
}

inline bool armv6_is_supported_vm_map(mmu_vm_map mvm) {
	bool ret = false;
	
	switch(mvm) {
		case MMU_4G_NO_SPLIT:
		case MMU_2G_2G_SPLIT:
		case MMU_1G_3G_SPLIT:
			ret = true;
			break;
	}
	
	return ret;
}

#endif
