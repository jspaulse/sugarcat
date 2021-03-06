/* Copyright (C) 2016 Jacob Paulsen <jspaulse@ius.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <arch/arm/armv7/armv7.h>
#include <arch/arm/cpu/cortex_a9.h>
#include <arch/arm/armv7/armv7_mmu.h>
#include <arch/arm/armv7/armv7_syscntl.h>

/* TODO: fix me */
#ifdef CONFIG_EARLY_KPRINTF
#include <mach/mach.h>
#endif

#include <arch/arch_mmu.h>
#include <init/kinit.h>
#include <util/atag.h>
#include <util/fdt.h>
#include <util/bits.h>
#include <mm/mem.h>
#include <mm/mm.h>
#include <memlayout.h>
#include <types.h>
#include <errno.h>
#include <stdbool.h>

#define PG_SZ		4096
#define PGD_ENTRY_SZ	4

#define MB		0x100000
#define MMU_PGTB_SIZE	0x200000
#define MMU_KPGD_SIZE	0x4000

#define MASK_MB 	0xFFFFF
#define DIV_MULT_MB 	20
#define DIV_MULT_PGTB	10
#define DIV_MULT_PG	12

/**
 * init_mmu_entry
 * 
 * defines features of initial
 * mmu entries
 *
 * @perm	permissions
 * @domain	domain
 * @flags	flags
 **/
struct init_mmu_entry {
    armv7_mmu_acc_perm	perms;
    unsigned char	domain;
    unsigned int	flags;
};

/* used for kinit_warn/info/print */
static char buf[512];

/* used for initial mappings */
static struct init_mmu_entry kdef_ent = {
    .perms	= ARMV7_MMU_ACC_KRW_NOU,
    .domain	= 0,
    .flags	= 0
};

extern void install_ivt();
/* helper functions */
static void move_high_sp(void);
static int init_get_mem(addr_t atag_fdt_base, struct mm_reg *reg);
static int init_get_mem_atag(addr_t atag_fdt_base, struct mm_reg *mem_reg);
static int init_get_mem_fdt(addr_t atag_fdt_base, struct mm_reg *mem_reg);
static int init_get_initrd(addr_t atag_fdt_base, struct mm_reg *mem_reg);
static int init_get_initrd_atag(addr_t atag_base, struct mm_reg *mem_reg);
static int init_setup_kern_pgtb(struct mm_reg *kern_pgd, struct mm_reg *kern_pgtb, struct init_mmu_entry *ent);
static int init_map_kern_pgtb(struct mm_reg *kern_pgtb, struct mm_reg *map_reg, struct init_mmu_entry *ent);
 
/**
 * vexpress_init
 * 
 * performs actual initializations of things required by the kernel 
 * and eventually branches into the kernel initialization.
 * 
 * this function:
 * performs sanity checks on the memory regions provided by either fdt or atag & kernel linker
 * moves the mmu mappings to page table mappings and maps the kernel & kernel init. regions
 * branches into the main kernel initialization
 **/
void vexpress_init(unsigned int mach, addr_t atag_fdt_base) {
    struct mm_reg	mmu_pgtb_reg	= {((kvm_to_phy((addr_t)&k_end) & ~MASK_MB) + MB), MMU_PGTB_SIZE};
    struct mm_vreg	mmu_pgtb_vreg	= {mmu_pgtb_reg.base, (((addr_t)&k_end & ~MASK_MB) + MB), mmu_pgtb_reg.size};
    struct mm_reg	mmu_pgd_reg	= {(addr_t)&k_pgd, MMU_KPGD_SIZE};
    struct mm_reg	kstack_reg	= {(kvm_to_phy((addr_t)&k_stack)) - PG_SZ, PG_SZ};
    struct mm_reg	kinit_reg	= {kvm_to_phy((addr_t)&hmi_start), ((size_t)&hmi_end - (size_t)&hmi_start)};
    struct mm_reg	kern_reg	= {kvm_to_phy((addr_t)&k_start), ((size_t)&k_end - (size_t)&k_start)};
    struct mm_reg	initrd_reg	= {0, 0};
    struct mm_reg	mem_reg 	= {0, 0};
    int			err		= 0;
    
    /* debug */
    #ifdef CONFIG_INIT_DEBUG
	mach_early_kprintf("========================= VEXPRESS_A9_QEMU "
	    "=========================\n");
	mach_early_kprintf("kernel init reg:\t0x%x\t0x%x\t%i bytes\n",
	    kinit_reg.base, kinit_reg.base + 
	    kinit_reg.size, kinit_reg.size);
	mach_early_kprintf("kernel phys reg:\t0x%x\t0x%x\t%i bytes\n",
	    kern_reg.base, kern_reg.base + 
	    kern_reg.size, kern_reg.size);
	mach_early_kprintf("kernel stack:\t\t0x%x\t0x%x\t%i bytes\n", 
	    kstack_reg.base, kstack_reg.base + 
	    kstack_reg.size, kstack_reg.size);
	mach_early_kprintf("mmu page dir reg:\t0x%x\t0x%x\t%i bytes\n",
	    mmu_pgd_reg.base, mmu_pgd_reg.base + mmu_pgd_reg.size, 
	    mmu_pgd_reg.size);
	mach_early_kprintf("mmu page table reg:\t0x%x\t0x%x\t%i bytes\n", 
	    mmu_pgtb_reg.base, mmu_pgtb_reg.base + mmu_pgtb_reg.size, 
	    mmu_pgtb_reg.size);
	    
	if (is_using_fdt(atag_fdt_base)) {
	    mach_early_kprintf("fdt base:\t\t0x%x\n", atag_fdt_base);
	} else if (is_using_atag(atag_fdt_base)) {
	    mach_early_kprintf("atag base:\t0x%x\n", atag_fdt_base);
	}
    #endif
    
    /* TODO: tmp */
    install_ivt();
    
    dump_fdt(atag_fdt_base);
    
    /* grab the largest continuous region of memory */
    if ((err = init_get_mem(atag_fdt_base, &mem_reg)) != ESUCC) {
	kinit_panic(buf, "init_get_mem() returned %i,"
	    "no defined memory regions available.", err);
    } else if (mem_reg.size <= 
	(kern_reg.size + kinit_reg.size + mmu_pgtb_reg.size)) {
	kinit_panic(buf, "not enough memory in region!  "
	    "init_get_mem() returned %i bytes in largest found region, "
	    "a minimum of %i bytes are required.", 
	    mem_reg.size, kinit_reg.size + 
	    kern_reg.size + mmu_pgtb_reg.size);
    }
	
    /* check if initrd exists */
    if (tag_exists(atag_fdt_base, ATAG_INITRD2)) {
	if ((err = init_get_initrd(atag_fdt_base, &initrd_reg)) != 0) {
	    kinit_warn(buf, "tag_exists(ATAG_INITRD2) returned true but init_get_initrd() returned %i; "
		"assuming it doesn't exist.", err);
	    //initrd_ex = false;
	} else if (initrd_reg.size == 0) {
	    kinit_warn(buf, "ATAG_INITRD2 exists but reports initrd.size == %i;"
		"assuming it doesn't exist.", initrd_reg.size);
	    //initrd_ex = false;
	} else {
	    //initrd_ex = true;
	}
    }
    
    /* clean (invalidate) pgtb region */
    memset((void *)mmu_pgtb_reg.base, 0, mmu_pgtb_reg.size);
    
    /* map kernel init */
    if ((err = init_map_kern_pgtb(&mmu_pgtb_reg, &kinit_reg, &kdef_ent)) != ESUCC) {
	kinit_warn(buf, "init_map_kern_pgtb(kinit_reg) failed with %i.", err);
    }
    
    /* map kernel */
    if ((err = init_map_kern_pgtb(&mmu_pgtb_reg, &kern_reg, &kdef_ent)) != ESUCC) {
	kinit_warn(buf, "init_map_kern_pgtb(kern_reg) failed with %i.", err);
    }
    
    /* map kernel stack */
    if ((err = init_map_kern_pgtb(&mmu_pgtb_reg, &kstack_reg, &kdef_ent)) != ESUCC) {
	kinit_warn(buf, "init_map_kern_pgtb(kstack_reg) failed with %i.", err);
    }
    
    /* map kernel pgtb */
    if ((err = init_map_kern_pgtb(&mmu_pgtb_reg, &mmu_pgtb_reg, &kdef_ent)) != ESUCC) {
	kinit_warn(buf, "init_map_kern_pgtb(mmu_pgtb_reg) failed with %i.", err);
    }
    
    /* map kernel pgd */
    if ((err = init_map_kern_pgtb(&mmu_pgtb_reg, &mmu_pgd_reg, &kdef_ent)) != ESUCC) {
	kinit_warn(buf, "init_map_kern_pgtb(mmu_pgd_reg) failed with %i.", err);
    }
    
    /* 
     * ensure that all data is written prior to adding entries
     * to active, operating mmu.
     */
    dsb();
    
    /* create pgd->pgtb mapping for kernel regions */
    if ((err = init_setup_kern_pgtb(&mmu_pgd_reg, &mmu_pgtb_reg, &kdef_ent)) != ESUCC) {
	kinit_panic(buf, "init_setup_kern_pgtb() failed with %i; nothing left to do.", err);
    }
    
    /* invalidate tlb */
    armv7_invalidate_unified_tlb();
    
    /* set up domains:
     * kernel domain does permission checking (client),
     * user domain does not.
     **/
    armv7_set_domain(USER_DOMAIN, ARMV7_DACR_MNGR);
    armv7_set_domain(KERN_DOMAIN, ARMV7_DACR_CLIENT);
    
    /* move sp into kernel memory */
    move_high_sp();
    
    struct fdt_node *test = fdt_get_node("chosen", atag_fdt_base);
    
    if (test != NULL) {
	struct fdt_property *initrd_end 	= fdt_get_property(atag_fdt_base, test, "linux,initrd-end");
	struct fdt_property *initrd_start 	= fdt_get_property(atag_fdt_base, test, "linux,initrd-start");
	
	if (initrd_end != NULL) {
	    if (initrd_start != NULL) {
		fdt32_t	*ptr = (fdt32_t *)initrd_end->data;
		    mach_early_kprintf("initrd_end: 0x%x\n", be32_to_cpu(*ptr));
		    
		    ptr = (fdt32_t *)initrd_start->data;
		    mach_early_kprintf("initrd_start: 0x%x\n", be32_to_cpu(*ptr));
		
		mach_early_kprintf("all bueno!\n");
	    }
	}
    } else {
	mach_early_kprintf("fdt_node null\n");
    }

    /* branch into kernel init */
    kernel_init(mach, atag_fdt_base, &mmu_pgtb_vreg, NULL, 0);
}

/**
 * init_get_mem
 * 
 * returns the first known region of memory (typically where
 * the kernel resides)
 * 
 * @atag_fdt_base	atag or fdt base
 * @mem_reg		return output
 * @return errno
 **/
static int init_get_mem(addr_t atag_fdt_base, struct mm_reg *mem_reg) {
    int ret = ESUCC;
    
    if (mem_reg != NULL) {
	if (is_using_fdt(atag_fdt_base)) {
	    ret = init_get_mem_fdt(atag_fdt_base, mem_reg);
	} else if (is_using_atag(atag_fdt_base)) {
	    ret = init_get_mem_atag(atag_fdt_base, mem_reg);
	} else {
	    ret = ENOTFND;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * init_get_mem_fdt
 * 
 * returns the first known region of memory (typically where the
 * kernel resides)
 * 
 * @atag_fdt_base	fdt base
 * @mem_reg		return output
 * @return errno
 **/
static int init_get_mem_fdt(addr_t atag_fdt_base, struct mm_reg *mem_reg) {
    struct fdt_node 	*node	= NULL;
    struct fdt_property *prop	= NULL;
    fdt32_t		*ptr	= NULL;
    int			ret	= ESUCC;
    
    /* memory@phy_base */
    sprintf(buf, "memory@0x%x\n", (addr_t)&kv_start);
    
    /* search for memory@phy_base */
    if ((node = fdt_get_node(buf, atag_fdt_base)) == NULL) {
	node = fdt_get_node("memory", atag_fdt_base);
	
	/* TODO: any additional range searches */
    }
    
    /* if we found our memory
     * node, get reg.
     */
    if (node != NULL) {
	if ((prop = fdt_get_property(atag_fdt_base, node, "reg")) != NULL) {
	    ptr = (fdt32_t *)prop->data;
	    
	    /* assign values */
	    mem_reg->base = be32_to_cpu(*ptr);
	    mem_reg->size = be32_to_cpu(*(ptr + 1));
	} else {
	    ret = ENOTFND;
	}
    } else {
	ret = ENOTFND;
    }
    
    return ret;
}

/**
 * init_get_mem_atag
 * 
 * returns the largest region of memory listed in the atags
 * 
 * this will attempt to coalesce any adjacent memory regions
 * 
 * @atag_fdt_base	atag base address
 * @mem_reg		struct to output into
 * @return errno
 **/
static int init_get_mem_atag(addr_t atag_fdt_base, struct mm_reg *mem_reg) {
    struct atag		*sch	= NULL;
    struct mm_reg	fnd 	= {0, 0};
    int 		ret 	= ESUCC;

    if (mem_reg != NULL) {
	sch = get_tag(atag_fdt_base, ATAG_MEM);
		
	if (sch != NULL) {
	    fnd.base	= sch->u.mem.start;
	    fnd.size	= sch->u.mem.sz;
			
	    while ((sch = get_next_tag(sch, ATAG_MEM)) != NULL) {
		if ((fnd.base + fnd.size) == sch->u.mem.start) { /* regions are continuous */
		    fnd.size += sch->u.mem.sz;
		} else if (fnd.size < sch->u.mem.sz) { /* larger region */
		    fnd.base	= sch->u.mem.start;
		    fnd.size	= sch->u.mem.sz;
		}
	    }
			
	    memcpy(mem_reg, &fnd, sizeof(struct mm_reg));
		
	} else {
	    ret = ENOTFND;
	}
    } else {
	ret = EINVAL;
    }
	
    return ret;
}

/* TODO: finish */
static int init_get_initrd(addr_t atag_fdt_base, struct mm_reg *mem_reg) {
    return init_get_initrd_atag(atag_fdt_base, mem_reg);
}
    
/**
 * init_get_initrd
 * 
 * returns the initrd memory region (if found)
 * 
 * @atag_base	atag base address
 * @reg		struct to output to
 * @return errno
 **/
static int init_get_initrd_atag(addr_t atag_base, struct mm_reg *mem_reg) {
    struct atag *sch	= NULL;
    int 	ret 	= 0;
    
    if (mem_reg != NULL) {
	sch = get_tag(atag_base, ATAG_INITRD2);
		
	if (sch != NULL) {
	    mem_reg->base = sch->u.initrd.start;
	    mem_reg->size = sch->u.initrd.sz;
	} else {
	    ret = ENOTFND;
	}
    } else {
	ret = EINVAL;
    }
	
    return ret;
}

/**
 * init_setup_kern_pgtb
 * 
 * creates the initial pgd->pgtb mapping for the whole kernel region.
 * this assumes that both kern_pgd & pgtb are continuous and 
 * kern_pgd->size >= 16KiB (which is required) & kern_pgtb->size >= (pgtb_cnt * PGTB_SIZE).
 * 
 * @kern_pgd	kernel page directory region
 * @kern_pgtb	kernel page table region
 * @ent		characteristics of entries
 * @pg_div_n	mmu division
 * @return errno
 **/
static int init_setup_kern_pgtb(struct mm_reg *kern_pgd, struct mm_reg *kern_pgtb, struct init_mmu_entry *ent) {
    addr_t		virt_addr	= 0;
    unsigned int 	pgtb_cnt	= 0; /* also, pgd_cnt == pgtb_cnt */
    unsigned int	i		= 0;
    int			pg_div_n	= armv7_mmu_get_pg_div();
    int			ret		= ESUCC;
    
    if (kern_pgd != NULL && kern_pgtb != NULL) {
	if (pg_div_n > 0) {
	    pgtb_cnt 	= PGD_ENTRY_CNT - (1 << (32 - (pg_div_n + DIV_MULT_MB)));
	    virt_addr	= (1 << (32 - pg_div_n));
	} else {
	    pgtb_cnt 	= PGD_ENTRY_CNT;
	    virt_addr	= 0x0;
	}
	
	/* create pgtb_cnt number of entries */
	while ((i < pgtb_cnt) && (ret == ESUCC)) {
	    struct armv7_mmu_pgd_entry pgd_ent = {
		.phy_addr	= kern_pgtb->base | (i << DIV_MULT_PGTB),
		.virt_addr	= virt_addr | (i << DIV_MULT_MB),
		.domain		= ent->domain,
		.type		= ARMV7_MMU_PGD_TABLE,
		.flags		= ent->flags		/* TODO */
	    };
	    
	    ret = armv7_mmu_map_pgd(&pgd_ent);
	    i++;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * init_map_kern_pgtb
 * 
 * maps kernel regions into specified kernel page table region.
 * this should be called prior to init_setup_kern_pgtb
 * 
 * @kern_pgtb	continuous physical region used for kernel page tables
 * @map_reg	region to map
 * @ent		characteristics of region
 * @pg_div_n	mmu division
 * @return errno
 **/
static int init_map_kern_pgtb(struct mm_reg *kern_pgtb, struct mm_reg *map_reg, struct init_mmu_entry *ent) {
    addr_t		pg_tb		= 0x0;
    addr_t		phy_addr	= 0x0;
    addr_t		virt_addr	= 0x0;
    addr_t		v_start		= 0x0;
    unsigned int	pg_cnt		= 0;
    unsigned int	i		= 0;
    int			pg_div_n	= armv7_mmu_get_pg_div();
    int 		ret 		= ESUCC;
    
    if (kern_pgtb != NULL && map_reg != NULL && ent != NULL) {
	pg_tb 		= kern_pgtb->base;
	virt_addr	= phy_to_kvm(map_reg->base);
	phy_addr	= map_reg->base;
	
	/* determines index */
	if (pg_div_n > 0) {
	    v_start	= (1 << (32 - (pg_div_n)));
	}
	
	/* get pg_cnt */
	if (!is_power_of_two(map_reg->size)) {
	    pg_cnt = clr_lv_msb(map_reg->size) >> (DIV_MULT_PG - 1);
	} else {
	    pg_cnt = clr_lv_msb(map_reg->size) >> DIV_MULT_PG;
	}
	
	while ((i < pg_cnt) && (ret == ESUCC)) {
	    int index = ((virt_addr - v_start) + (i << DIV_MULT_PG)) >> DIV_MULT_MB;
	    
	    /* entry */
	    struct armv7_mmu_pgtb_entry pgtb_ent = {
		.phy_addr	= phy_addr | (i << DIV_MULT_PG),
		.virt_addr	= virt_addr | (i << DIV_MULT_PG),
		.acc_perm	= ent->perms,
		.type		= ARMV7_MMU_PGTB_SMALL_PG,
		.flags		= ent->flags			/* TODO */
	    };
	    
	    ret = armv7_mmu_map_new_pgtb((pg_tb | (index << DIV_MULT_PGTB)), &pgtb_ent);
	    i++;
	}
    } else {
	ret = EINVAL;
    }
    
    return ret;
}

/**
 * move_high_sp
 * 
 * moves the stack pointer into higher, memory mapped
 * region
 **/
static void move_high_sp(void) {
    addr_t sp = 0;
	
    /* get sp */
    asm volatile("mov %0, sp" : "=r" (sp));
	
    /* get high virt. address */
    sp = phy_to_kvm(sp);

    /* now, assign it back */
    asm volatile ("mov sp, %0" : : "r" (sp));
}

