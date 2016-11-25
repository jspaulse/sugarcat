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
#include <arch/arm/armv7/armv7_mmu.h>
#include <arch/arm/armv7/armv7_syscntl.h>
#include <arch/arm/cpu/cortex_a9.h>
#include <mach/vexpress_a9/vexpress_a9.h>
#include <mach/mach_init.h>
#include <init/kinit.h>
#include <util/atag.h>
#include <util/bits.h>
#include <mm/mem.h>
#include <mm/mm.h>
#include <types.h>
#include <errno.h>
#include <stdbool.h>

#define PG_SZ		4096
#define PGTB_SZ		0x400
#define PGD_ENTRY_CNT	4096
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

/* temp */
extern void install_ivt(void);

/* helper functions */
static void move_high_sp(void);
static int init_get_mem(addr_t atag_base, struct mm_reg *reg);
static int init_get_initrd(addr_t atag_base, struct mm_reg *reg);
static int init_setup_kern_pgtb(struct mm_reg *kern_pgd, struct mm_reg *kern_pgtb, struct init_mmu_entry *ent, int pg_div_n);
static int init_map_kern_pgtb(struct mm_reg *kern_pgtb, struct mm_reg *map_reg, struct init_mmu_entry *ent, int pg_div_n);

/**
 * vexpress_init
 * 
 * performs actual initializations of things required by the kernel 
 * and eventually branches into the kernel initialization
 * 
 * (TODO) describe in light detail what we're doing
 **/
void vexpress_init(unsigned int mach, addr_t atag_base) {
    size_t		bss_sz		= (size_t)&bss_end - (size_t)&bss_start;
    struct mm_reg	kern_reg	= {init_kvm_to_phy((addr_t)&k_start), ((size_t)&k_end - (size_t)&k_start)};
    struct mm_reg	mmu_pgtb_reg	= {(init_kvm_to_phy((addr_t)&k_end) & ~MASK_MB) + MB, MMU_PGTB_SIZE};
    struct mm_reg	kstack_reg	= {(init_kvm_to_phy((addr_t)&kern_stack)) - PG_SZ, PG_SZ};
    struct mm_reg	mmu_pgd_reg	= {(addr_t)&k_pgd, MMU_KPGD_SIZE};
    struct mm_reg	initrd_reg	= {0, 0};
    struct mm_reg	mem_reg 	= {0, 0};
    bool		initrd_ex	= false;
    int			pg_div		= 0;
    int			err		= 0;
	
    /*
    struct mm_reg *reserved_regs[] = {
	&kern_reg,
	&mem_reg,
	&initrd
    };
    */
    
    memset(&bss_start, 0, bss_sz);

    /* grab the largest continuous region of memory */
    if ((err = init_get_mem(atag_base, &mem_reg)) != ERR_SUCC) {
	kinit_panic(buf, "init_get_mem() returned %i, no defined memory regions available.", err);
    } else if (mem_reg.size <= (kern_reg.size + mmu_pgtb_reg.size)) {
	kinit_panic(buf, "not enough memory in region!  init_get_mem() returned %i bytes \
	    in largest found region, a minimum of %i bytes are required.", mem_reg.size, kern_reg.size + mmu_pgtb_reg.size);
    }
	
    /* check if initrd exists */
    if (tag_exists(atag_base, ATAG_INITRD2)) {
	if ((err = init_get_initrd(atag_base, &initrd_reg)) != 0) {
	    kinit_warn(buf, "tag_exists(ATAG_INITRD2) returned true but init_get_initrd() returned %i; \
		assuming it doesn't exist.", err);
	    initrd_ex = false;
	} else if (initrd_reg.size == 0) {
	    kinit_warn(buf, "ATAG_INITRD2 exists but reports initrd.size == %i; \
		assuming it doesn't exist.", initrd_reg.size);
	    initrd_ex = false;
	} else {
	    initrd_ex = true;
	}
    }
    
    /* sanity check on reservations */
    if (!is_within_region(&mem_reg, &kern_reg)) {
	kinit_warn(buf, "is_within_region() reports kernel region is not in largest found memory region; \
	    k_base: 0x%x, k_size: %i, m_base: 0x%x, m_size: %i.", kern_reg.base, kern_reg.size, mem_reg.base, mem_reg.size);
    } 
	
    /* check if mmu pte is in sane region */
    if (!is_within_region(&mem_reg, &mmu_pgtb_reg)) {
	kinit_warn(buf, "is_within_region() reports mmu_pgtb region is not in largest found memory region; \
	    pgtb_base: 0x%x, pgtb_size: %i, m_base: 0x%x, m_size: %i.", mmu_pgtb_reg.base, mmu_pgtb_reg.size, mem_reg.base, mem_reg.size);
    }
	
    /* check if overlapping with initrd */
    if (initrd_ex && is_overlapping(&initrd_reg, &mmu_pgtb_reg)) {
	kinit_warn(buf, "is_overlapping() reports mmu_pgtb is overlapping the initrd; pgtb_base: 0x%x, pgtb_size: %i, \
	    initrd_base: 0x%x, initrd_size: %i.", mmu_pgtb_reg.base, mmu_pgtb_reg.size, initrd_reg.base, initrd_reg.size);
    }
    
    /* bring up post init mmu */
    if ((err = armv7_mmu_init_post_enable()) != ERR_SUCC) {
	kinit_panic(buf, "armv7_init_post_enable() failed with %i; somehow we've made it this far without \
	    enabling the mmu?", err);
    }
    
    /* clean (invalidate) pgtb region */
    memset((void *)mmu_pgtb_reg.base, 0, mmu_pgtb_reg.size);
    
    /* temp */
    install_ivt();
    
    /* grab set pg_div */
    pg_div = armv7_mmu_get_pg_div();
    
    /* map kernel */
    if ((err = init_map_kern_pgtb(&mmu_pgtb_reg, &kern_reg, &kdef_ent, pg_div)) != ERR_SUCC) {
	kinit_warn(buf, "init_map_kern_pgtb(kern_reg) failed with %i.", err);
    }
    
    /* map kernel stack */
    if ((err = init_map_kern_pgtb(&mmu_pgtb_reg, &kstack_reg, &kdef_ent, pg_div)) != ERR_SUCC) {
	kinit_warn(buf, "init_map_kern_pgtb(kstack_reg) failed with %i.", err);
    }
    
    /* 
     * ensure that all data is written prior to adding entries
     * to active, operating mmu
     */
    dsb();
    
    /* create pgd->pgtb mapping for kernel regions */
    if ((err = init_setup_kern_pgtb(&mmu_pgd_reg, &mmu_pgtb_reg, &kdef_ent, pg_div)) != ERR_SUCC) {
	kinit_panic(buf, "init_setup_kern_pgtb() failed with %i; nothing left to do.", err);
    }
    
    /* move sp into kernel memory */
    move_high_sp();
    
    /* do preliminary interrupt enabling (?) */
    /* full fledged interrupt enabling (?)	*/
	
    /* enable, bring up scu */
    /* allocate stacks for additional cpu's
     * we only need them temporarily;
     * once they are in the holding pen
     * they will only leave once we have a task
     * or they recv an interrupt 
     */
    
    /* bring cpu's up into holding pen */
    /* set their mmu's to k_pgd
     * (seems like we should've done this 
     * in low init)
     * (this can be arch/arm/armv7/cpus) */
    /* (this also means we'll need to define the private interrupts */
    /* for stuff and things) */
	
    if (mach == 0) {
		
    }
	
	
    while(1);
}

/**
 * init_get_mem
 * 
 * returns the largest region of memory listed in the atags
 * 
 * this will attempt to coalesce any adjacent memory regions
 * 
 * @atag_base	atag base address
 * @reg		struct to output into
 * @return errno
 **/
static int init_get_mem(addr_t atag_base, struct mm_reg *reg) {
    struct atag		*sch	= NULL;
    struct mm_reg	fnd 	= {0, 0};
    int 		ret 	= ERR_SUCC;

    if (reg != NULL) {
	sch = get_tag(atag_base, ATAG_MEM);
		
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
			
	    memcpy(reg, &fnd, sizeof(struct mm_reg));
		
	} else {
	    ret = ERR_NOTFND;
	}
    } else {
	ret = ERR_INVAL;
    }
	
    return ret;
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
static int init_get_initrd(addr_t atag_base, struct mm_reg *reg) {
    struct atag 	*sch	= NULL;
    int 		ret 	= 0;
	
    if (reg != NULL) {
	sch = get_tag(atag_base, ATAG_INITRD2);
		
	if (sch != NULL) {
	    reg->base = sch->u.initrd.start;
	    reg->size = sch->u.initrd.sz;
	} else {
	    ret = ERR_NOTFND;
	}
    } else {
	ret = ERR_INVAL;
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
static int init_setup_kern_pgtb(struct mm_reg *kern_pgd, struct mm_reg *kern_pgtb, struct init_mmu_entry *ent, int pg_div_n) {
    addr_t		virt_addr	= 0;
    unsigned int 	pgtb_cnt	= 0; /* also, pgd_cnt == pgtb_cnt */
    int			ret		= 0;
    unsigned int	i		= 0;
    
    if (kern_pgd != NULL && kern_pgtb != NULL) {
	if (pg_div_n > 0) {
	    pgtb_cnt 	= PGD_ENTRY_CNT - (1 << (32 - (pg_div_n + DIV_MULT_MB)));
	    virt_addr	= (1 << (32 - pg_div_n));
	} else {
	    pgtb_cnt 	= PGD_ENTRY_CNT;
	    virt_addr	= 0x0;
	}
	
	/* create pgtb_cnt number of entries */
	while ((i < pgtb_cnt) && (ret == ERR_SUCC)) {
	    struct armv7_mmu_pgd_entry pgd_ent = {
		.phy_addr	= kern_pgtb->base | (i << DIV_MULT_PGTB),
		.virt_addr	= virt_addr | (i << DIV_MULT_MB),
		.domain		= ent->domain,
		.type		= ARMV7_MMU_PGD_TABLE,
		.flags		= ent->flags		/* TODO */
	    };
	    
	    ret = armv7_mmu_map_pgd(&pgd_ent);
	    i++;
	    
	    //mach_init_printf("init_setup_kern_pgtb[%i] pgtb_cnt: %i, p: 0x%x, v: 0x%x\n", (i - 1), pgtb_cnt, pgd_ent.phy_addr, pgd_ent.virt_addr);
	}
    } else {
	ret = ERR_INVAL;
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
static int init_map_kern_pgtb(struct mm_reg *kern_pgtb, struct mm_reg *map_reg, struct init_mmu_entry *ent, int pg_div_n) {
    addr_t		pg_tb		= 0x0;
    addr_t		phy_addr	= 0x0;
    addr_t		virt_addr	= 0x0;
    unsigned int	pg_cnt		= 0;
    unsigned int	pgtb_cnt	= 0;	/* number of pgtb in pg div */
    unsigned int	i		= 0;
    int 		ret 		= ERR_SUCC;
    
    if (kern_pgtb != NULL && map_reg != NULL && ent != NULL) {
	pg_tb 		= kern_pgtb->base;
	virt_addr	= init_phy_to_kvm(map_reg->base);
	phy_addr	= map_reg->base;
	
	/* determines index */
	if (pg_div_n > 0) {
	    pgtb_cnt 	= PGD_ENTRY_CNT - (1 << (32 - (pg_div_n + DIV_MULT_MB)));
	} else {
	    pgtb_cnt 	= PGD_ENTRY_CNT;
	}
	
	/* get pg_cnt */
	if (!is_power_of_two(map_reg->size)) {
	    pg_cnt = clr_lv_msb(map_reg->size) >> (DIV_MULT_PG - 1);
	} else {
	    pg_cnt = clr_lv_msb(map_reg->size) >> DIV_MULT_PG;
	}
	
	while ((i < pg_cnt) && (ret == ERR_SUCC)) {
	    int index = pgtb_cnt - ((virt_addr | (i << DIV_MULT_PG)) >> DIV_MULT_MB);
	    
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
	    
	    mach_init_printf("init_map_kern_pgtb[%i] pg_tb: 0x%x, p: 0x%x, v: 0x%x, index: %i\n", (i - 1), (pg_tb | (index << DIV_MULT_PGTB)), pgtb_ent.phy_addr, pgtb_ent.virt_addr, index);
	}
    } else {
	ret = ERR_INVAL;
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
    sp = init_phy_to_kvm(sp);

    /* now, assign it back */
    asm volatile ("mov sp, %0" : : "r" (sp));
}

