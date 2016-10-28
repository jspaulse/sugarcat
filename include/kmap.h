#ifndef KMAP_H
#define KMAP_H
#include <mm/mmu.h>
#include <mm/pmm.h>

#define MMU_KPGDIR	0x4000
#define MMU_UPGDIR	0x6000
#define MMU_PGD_SZ	0x2000
#define SVC_STACK	0x2000

/* kernel.ld */
extern unsigned int kv_start;
extern unsigned int ss_start, ss_end;
extern unsigned int ss_bss_start, ss_bss_end;
extern unsigned int bss_start, bss_end;
extern unsigned int k_start, k_end;
extern unsigned int svc_stack;

/* kmap.c */
extern const mmu_vm_map kmap_mvm;
extern const struct mm_region kmap_regions[];
extern const int kmap_regions_cnt;

#endif
