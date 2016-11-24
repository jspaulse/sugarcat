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
.extern kern_stack		/* kernel.ld */

/* armv7_ivt.c */
.extern armv7_irq_hand
/* temp */
.extern dump_undef_except
.extern dump_data_abt
.extern dump_pref_abt

.global armv7_irq_hand
armv7_irq_hand:
	ldr sp, =kern_stack
	add sp, sp, #0x2000
	
	push {r0-r3}
	
	/* branch into C handler */
	bl c_irq_hand
	
	pop {r0-r3}
	
	/* return to before interrupt */
	subs pc, lr, #4

.global armv7_undef_hand
armv7_undef_hand:
	ldr sp, =kern_stack
	add sp, sp, #0x3000
	
	push {r0-r3}
	
	mov r0, lr
	sub r0, r0, #4
	
	/* lr - 4 holds undefined exception */
	bl dump_undef_except
	
	pop {r0-r3}
	cps #0x1B
	
	add lr, lr, #4
	movs pc, r14

.global armv7_dat_abt_hand
armv7_dat_abt_hand:
	ldr sp, =kern_stack
	add sp, sp, #0x3000
	
	push {r0-r3}
	
	mov r0, lr
	sub r0, r0, #8
	
	bl dump_data_abt
	
	pop {r0-r3}
	
	/* returns after */
	subs pc, lr, #4

.global armv7_pref_abt_hand
armv7_pref_abt_hand:
	ldr sp, =kern_stack
	add sp, sp, #0x3000
	
	push {r0-r3}
	
	mov r0, lr
	sub r0, r0, #4
	
	bl dump_pref_abt
	
	pop {r0-r3}
	
	/* returns after */
	movs pc, lr
