#ifndef ARMV6_H
#define ARMV6_H

/* CPSR */
#define IRQ_DIS 	0x40
#define FIQ_DIS 	0x80
#define ABT_DIS		0x100
#define MODE_USR	0x10
#define MODE_FIQ	0x11
#define MODE_IRQ	0x12
#define MODE_SVC	0x13
#define MODE_ABT	0x17
#define MODE_UND	0x1B
#define MODE_SYS	0x1F


inline void armv6_set_domain(unsigned int x) {
	asm("mcr p15, 0, %[v], c3, c0, 0": :[v]"r" (x):);
}

inline unsigned int armv6_get_domain(void) {
	unsigned int ret = 0;	//unsigned int ret = 0;

	asm("mrc p15, 0, %[r], c3, c0, 0": [r]"=r" (ret)::);
	return ret;
}

inline void armv6_set_ttbcr(unsigned int x) {
	asm("MCR p15, 0, %[v], c2, c0, 2": :[v]"r" (x):);
}

inline void armv6_set_ttb0(unsigned int x) {
	asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (x):);
}

inline void armv6_set_ttb1(unsigned int x) {
	asm("MCR p15, 0, %[v], c2, c0, 1": :[v]"r" (x):);
}

inline unsigned int armv6_read_cntl_reg(void) {
	unsigned int ret = 0;
	asm("MRC p15, 0, %[r], c1, c0, 0": [r]"=r" (ret)::);
	
	return ret;
}

inline void armv6_write_cntl_reg(unsigned int x) {
	asm("MCR p15, 0, %[r], c1, c0, 0": :[r]"r" (x):);	
}

inline void armv6_invalidate_tlbs(void) {
	unsigned int x = 0;
	asm("MCR p15, 0, %[r], c8, c7, 0" : :[r]"r" (x):);
}


#endif

