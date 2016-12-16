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
#include <mach/mach_init.h>
#include <init/kinit.h>
#include <mm/mem.h>
#include <linker.h>
#include <types.h>

void kernel_init(unsigned int mach, addr_t atag_fdt_base, struct mm_vreg *mmu_pgtb_reg, struct mm_vreg *reserved_regs, int reg_cnt) {
    size_t hmi_bss_sz		= (size_t)&hmi_bss_start - (size_t)&hmi_bss_end;
    size_t k_bss_sz		= (size_t)&k_end - (size_t)&k_start;
    
    /* clear hmi & kernel bss */
    memset(&hmi_bss_start, 0, hmi_bss_sz);
    memset(&k_start, 0, k_bss_sz);
    
    /* we're not provided with anything for init; panic */
    //if (!is_using_fdt(atag_fdt_base) && !is_using_atag(atag_fdt_base)) {
	//kinit_panic(buf, "kernel was not provided fdt or atag; init. is impossible to do without either.\n", 0);
    //}
    
    if (mach) {
	if (atag_fdt_base) {
	    if (mmu_pgtb_reg) {
		if (reserved_regs) {
		    if (reg_cnt) {
			
		    }
		}
	    }
	}
    }
}


