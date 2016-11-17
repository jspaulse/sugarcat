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
#include <stdbool.h>
#include <stdarg.h>
#include <mm/mem.h>
#include <util/str.h>

#define UART0_BASE_ADDRESS		0x10009000
//#define UART0_BASE_ADDRESS		0x101f1000
#define UART0_FR				0x18



static void uart_putc(unsigned char byte) {
	/* not required qemu
	while(true) {
		if (!(memr(UART0_BASE_ADDRESS + UART0_FR) & (1 << 5))) {
			break;
		}
	}
	*/
	memw(UART0_BASE_ADDRESS, (unsigned int)byte);
}

static void uart_puts(char *buf) {
	int i = 0;
	
	//dsb(); not required atm, qemu
	
	while (buf[i] != '\0') {
		uart_putc(buf[i++]);
	}
}

void mach_init_printf(const char *format, ...) {
	char buf[11];
	unsigned int index = 0;
	va_list va;
	
	if (format != NULL) {
		va_start(va, format);
		
		while (format[index] != '\0') {
			switch(format[index]) {
				case '%' :
					index++;
				
					if (format[index] == 'x') {
						memset(buf, 0, sizeof(buf));
						uart_puts(itox(va_arg(va, int), buf));
					} else if (format[index] == 'i') {
						memset(buf, 0, sizeof(buf));
						uart_puts(itoa(va_arg(va, int), buf));
					} else if (format[index] == 's') {
						uart_puts(va_arg(va, char*));
					} else if (format[index] == 'c') {
						uart_putc(va_arg(va, int));
					}
					break;
				default:
					uart_putc(format[index]);
					break;
			}
			
			index++;
		}
		
		va_end(va);
	}
}
