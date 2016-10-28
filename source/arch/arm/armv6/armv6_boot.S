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
//#include <kmap.h>
.global reset

.extern svc_stack
.extern early_init

reset:
	/* move into svc mode, set sp */
	msr cpsr_c, #0xD3 /* (MODE_SVC | IRQ_DIS | FIQ_DIS) */
	ldr sp, =svc_stack
	
	//stmfd sp!, {r0, r1, r2} /* push three initial registers */
    //ldmfd sp!, {r0, r1, r2} /* pop three initial registers */
    
    /* branch into init */
    b early_init
    
    /* hang if we ever return here (we won't) */
    hang:
		b hang

.global set_sp
set_sp:
	mov sp, r0
	mov pc, lr

.global get_sp
get_sp:
	mov r0, sp
	bx lr

.global add_sp
add_sp:
	add sp, sp, r0
	mov pc, lr
