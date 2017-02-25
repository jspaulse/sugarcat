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
#include <util/bits.h>
#include <util/fdt.h>
#include <util/str.h>
#include <memlayout.h>
#include <errno.h>
#include <mach/mach.h> /* TODO: tmp */

/* functionality for grabbing memory size, memory start */
/* functionality for grabbing initrd */
/* functionality for grabbing kernel pgtb region & size
 * this is probably a job for device tree
 */

/**
 * mlay_get_phy_mem_reg
 * Memory Layout Get Physical Memory Region
 * 
 * returns the first known physical memory region (from the fdt) where
 * the kernel resides
 * 
 * @fdt_base	base address of fdt
 * @mem_reg	returned memory region
 * @return errno
 **/
int mlay_get_phy_mem_reg(addr_t fdt_base, struct mm_reg *mem_reg) {
    struct fdt_node 	*node	= NULL;
    struct fdt_property *prop	= NULL;
    fdt32_t		*ptr	= NULL;
    addr_t		p_start	= mlay_get_kern_phy_start();
    int			ret	= ESUCC;
    char		buf[32];
    
    /* memory@phy_base */
    sprintf(buf, "memory@0x%x\n", p_start);
    
    /* search for memory@phy_base */
    if ((node = fdt_get_node(buf, fdt_base)) == NULL) {
	node = fdt_get_node("memory", fdt_base);
	
	/* TODO: any additional range searches */
    }
    
    /* if we found our memory node, get reg */
    if (node != NULL) {
	if ((prop = fdt_get_property(fdt_base, node, "reg")) != NULL) {
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

int mlay_get_initrd_reg(addr_t fdt_base, struct mm_reg *initrd_reg) {
    struct fdt_node	*node	= NULL;
    struct fdt_property	*s_prop	= NULL;
    struct fdt_property	*e_prop	= NULL;
    int 		ret 	= ESUCC;
    
    /* grab the "chosen" node */
    if ((node = fdt_get_node("chosen", fdt_base)) != NULL) {
	if ((s_prop = fdt_get_property(fdt_base, node, "linux,initrd-start")) != NULL) { /* using linux,initrd-xxx form */
	    if ((e_prop = fdt_get_property(fdt_base, node, "linux,initrd-end")) != NULL) {
		mach_early_kprintf("0x%x, s_prop->data: 0x%x\n", s_prop->data, be32_to_cpu(*s_prop->data));
		mach_early_kprintf("0x%x, e_prop->data: 0x%x\n", e_prop->data, be32_to_cpu(*e_prop->data));
		initrd_reg->base = be32_to_cpu(*s_prop->data);
		initrd_reg->size = be32_to_cpu(*e_prop->data) - be32_to_cpu(*s_prop->data);
	    } else {
		ret = ENOTFND;
	    }
	} else if ((s_prop = fdt_get_property(fdt_base, node, "initrd-start")) != NULL) { /* using initrd-xxx form */
	    
	} else {
	    ret = ENOTFND;
	}
    } else {
	ret = ENOTFND;
    }
    
    return ret;
}
/*
int mlay_get_initrd
* linux,initrd-end
		linux,initrd-start
* initrd-start = <0xc8000000>;
		initrd-end = <0xc8200000>;
*/
