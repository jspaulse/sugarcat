#ifndef MMU_H
#define MMU_H
#include <types.h>
#include <stdbool.h>
/* errors */

#define MMU_ERR_NO_PAGE_DIRS		-3
#define MMU_ERR_ALREADY_ENABLED		-2
#define MMU_ERR_NOT_ENABLED			-1
#define MMU_ERR_SUCCESS				0
#define MMU_ERR_INVALID_ENTRY		1
#define MMU_ERR_INVALID_ACCESS		2
#define MMU_ERR_UNSUPPORTED_VM_MAP	3
#define MMU_ERR_NO_PAGE_TABLE		4


// entry flags
#define MMU_CACHED					0x4

typedef enum {
	MMU_4G_NO_SPLIT,
	MMU_1G_3G_SPLIT,
	MMU_2G_2G_SPLIT
} mmu_vm_map;

typedef enum {
	PG_DIR_ENTRY			= 0x1,
	PG_DIR_TABLE			= 0x3,
	PG_DIR_TRANS_FAULT		= 0x7,
	PG_TABLE_ENTRY			= 0x4,
	PG_TABLE_TRANS_FAULT	= 0xC
} pg_ent_t;

typedef enum {
	KERN_RW,
	USER_RW 	/* implicit kernel rw */
} pg_acc_t;

struct mmu_entry {
	addr_t			pg_dir;
	addr_t			vaddr;
	addr_t			paddr;
	pg_ent_t		entry;
	pg_acc_t		access;
	unsigned char	flags;
};

inline addr_t mmu_get_high_vaddr(mmu_vm_map mvm) {
	addr_t ret = 0;
	
	switch(mvm) {
		case MMU_2G_2G_SPLIT:
			ret = 1 << 31;
			break;
		case MMU_1G_3G_SPLIT:
			ret = 1 << 30;
			break;
		default:
			ret = 0;
			break;
	}
	
	return ret;
}

inline bool mmu_addr_is_high_mem(mmu_vm_map mvm, addr_t addr) {
	bool ret = false;
	
	if (addr >= mmu_get_high_vaddr(mvm)) {
		ret = true;
	}
	
	return ret;
}

int mmu_enable(void);
int mmu_disable(void);
int mmu_set_kern_pg_dir(addr_t pgd_addr, bool invalidate);
int mmu_set_user_pg_dir(addr_t pgd_addr, bool invalidate);
int mmu_set_pg_dirs(addr_t kpgd, addr_t upgd, bool invalidate);
int mmu_set_vm_map(mmu_vm_map mvm, bool update);
int mmu_map(addr_t vaddr, addr_t paddr, pg_ent_t ent, pg_acc_t acc, unsigned char flags, bool invalidate);
int mmu_map_new(addr_t pg_dir, addr_t vaddr, addr_t paddr, pg_ent_t ent, pg_acc_t acc, unsigned char flags);
#endif


