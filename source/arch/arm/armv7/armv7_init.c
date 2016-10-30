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
 
extern void d_printf(const char *format, ...);

void armv7_init(int r0, int mach, unsigned int atags) {
	if (r0 == 0) {
		if (mach == 0) {
			if (atags == 0) {
			
			}
		}
	}
	
	/* basically what needs to happen is this:
	 * 
	 * clear the start bss
	 * 
	 * set up both tables (2:2 split) with mappings in both pointing to
	 * where the kernel is.
	 * 
	 * next, branch into the higher kernel (stack needs to be added + kv_start) kmain
	 * 
	 * in kmain we'll clear the bss, set the ivt up (at the bottom of our address space)
	 * pointing at our handlers (might be better to do this initially so we can watch our mappings)
	 * (move debug into early boot)
	 * 
	 * 
	 **/
	
	//d_printf("Testing testing test!\n");
	return;
}
