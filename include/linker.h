#ifndef LINKER_H
#define LINKER_H
#include <types.h>

/**
 * linker.h provides the regions defined the kernel.ld linker file
 * used in compilation
 **/
extern addr_t kp_start, kv_start;		/* kern. virt/phy start */
extern addr_t lmi_start, lmi_end;		/* low memory init */
extern addr_t lmi_bss_start, lmi_bss_end;	/* low memory bss */
extern addr_t k_pgd;				/* kernel page dir */
extern addr_t k_stack;				/* kernel stack */
extern addr_t hmi_start, hmi_end;		/* high memory init */
extern addr_t hmi_bss_start, hmi_bss_end;	/* high memory init bss */
extern addr_t k_start, k_end;			/* kernel region */
extern addr_t k_bss_start, k_bss_end;		/* kernel bss */

#endif
