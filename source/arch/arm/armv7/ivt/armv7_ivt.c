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
#include <arch/arm/armv7/armv7.h>
#include <arch/arm/armv7/armv7_syscntl.h>

extern void armv7_irq_hand(void);
extern void armv7_undef_hand(void);
extern void armv7_dat_abt_hand(void);
extern void armv7_pref_abt_hand(void);

static unsigned int ivt[16] __attribute__((aligned(128)));
/*
    ARMV7_LDR_PC | 0x18,
    ARMV7_LDR_PC | 0x18,
    ARMV7_LDR_PC | 0x18,
    ARMV7_LDR_PC | 0x18,
    ARMV7_LDR_PC | 0x18,
    ARMV7_LDR_PC | 0x18,
    ARMV7_LDR_PC | 0x18,
    ARMV7_LDR_PC | 0x18,
    0,
    (unsigned int)armv7_undef_hand,
    4,
    (unsigned int)armv7_pref_abt_hand,
    (unsigned int)armv7_dat_abt_hand,
    8,
    (unsigned int)armv7_irq_hand,
    16
};
*/
void install_ivt(void) {
	for (int i = 0; i < 8; i++) {
		ivt[i] = ARMV7_LDR_PC | 0x18;
	}
	
	ivt[8]		= (unsigned int)0;
	ivt[9]		= (unsigned int)armv7_undef_hand;
	ivt[10]		= (unsigned int)4;
	ivt[11]		= (unsigned int)armv7_pref_abt_hand;
	ivt[12]		= (unsigned int)armv7_dat_abt_hand;
	ivt[13]		= (unsigned int)16;
	ivt[14]		= (unsigned int)armv7_irq_hand;
	ivt[15]		= (unsigned int)20;
	
	mach_init_printf("install_ivt()\n");
	/* lastly, set the vector base */
	armv7_set_ivt_base((unsigned int)&ivt);
}

void dump_undef_except(unsigned int addr) {
	mach_init_printf("[ERROR] Undefined Instruction at 0x%x\n", addr);
}

void dump_data_abt(unsigned int addr) {
	unsigned int type = 0;
	
	asm volatile ("mrc p15, 0, %0, c5, c0, 0\n" : "=r" (type));
	mach_init_printf("[ERROR] Data Abort at 0x%x, reason: 0x%x\n", addr, type);
}

void dump_pref_abt(unsigned int addr) {
	unsigned int type = 0;
	
	asm volatile("mrc p15, 0, %0, c5, c0, 1" : "=r" (type));
	
	mach_init_printf("[ERROR] Prefetch Abort at 0x%x, FSR[10]: 0x%x, FSR[3:0]: 0x%x\n", addr, ((type >> 10) & 0x1), type & 0xF);
	mach_init_printf("register: 0x%x\n", type);
	
}

void c_irq_hand(void) {
    /*
	unsigned int base = armv7_get_config_base() + ARM_GICC_OFFSET;
	unsigned int irq_ack = memr(base + 0xC) & 0x1FFF;
	
	mach_init_printf("Interrupt!  ID: 0x%x\n", (irq_ack & 0x3FF));
	
	// write and mark it finished 
	memw(base + 0x10, irq_ack);
	*/
}

/*
 * armv7_irq_hand
armv7_undef_hand
armv7_dat_abt_hand
armv7_pref_abt_hand
*/
