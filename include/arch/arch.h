#ifndef ARCH_H
#define ARCH_H

void dmb();
void dsb();

int set_kern_stack(unsigned int address);
int set_irq_stack(unsigned int address);
int set_fiq_stack(unsigned int address);
int set_abt_stack(unsigned int address);
int set_und_stack(unsigned int address);
#endif
