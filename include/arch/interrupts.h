#ifndef INTERRUPTS_H
#define INTERRUPTS_H

extern void irq_enable(void);
extern void irq_disable(void);
extern void fiq_enable(void);
extern void fiq_disable(void);
extern void abt_enable(void);
extern void abt_disable(void);
#endif


