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
#include <mm/mem.h>
#define PERI_MAILBOX_ADDR 		0x2000B880
#define MAILBOX_READ    		0x0
#define MAILBOX_PEEK    		0x10
#define MAILBOX_SENDER  		0x14
#define MAILBOX_STATUS  		0x18
#define MAILBOX_CONFIG  		0x1C
#define MAILBOX_WRITE   		0x20
#define MAILBOX_FULL			0x80000000
#define MAILBOX_EMPTY			0x40000000
#define MASK_MAILBOX_CHANNEL 	0x0000000F
#define MASK_MAILBOX_DATA 		0xFFFFFFF0
#define PROPERTY_TAG_ARM_TO_VC  8
#define TAG_GET_ARM_MEMORY		0x00010005

extern void d_printf(const char *format, ...);

static unsigned int mbox_status(void) {
	return memr(PERI_MAILBOX_ADDR + MAILBOX_STATUS);
}

static unsigned int mbox_read(unsigned int channel) {
	unsigned int ret = 0;
	
	do {
		while ((mbox_status() &  MAILBOX_EMPTY));

		ret = memr(PERI_MAILBOX_ADDR + MAILBOX_READ);
	} while ((ret & MASK_MAILBOX_CHANNEL) != channel);
	
	dmb();
	return ret & ~MASK_MAILBOX_CHANNEL;
}

static void mbox_write(unsigned int data, unsigned int channel) {
	dsb();
	
	while ((mbox_status() & MAILBOX_FULL));
	memw(PERI_MAILBOX_ADDR + MAILBOX_WRITE, ((data & MASK_MAILBOX_DATA) | channel));
}

size_t mach_get_phy_mem_size() {
	unsigned int mailbuf[16] __attribute__((aligned(16)));
	
	mailbuf[0] = 8 * 4;
	mailbuf[1] = 0;
	mailbuf[2] = TAG_GET_ARM_MEMORY;
	mailbuf[3] = 8;
	mailbuf[4] = 0;
	mailbuf[5] = 0;
	mailbuf[6] = 12345;
	mailbuf[7] = 0;
	
	
	mbox_write((unsigned int)&mailbuf, PROPERTY_TAG_ARM_TO_VC);
	mbox_read(PROPERTY_TAG_ARM_TO_VC);
	
	return mailbuf[6];
}
