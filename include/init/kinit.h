#ifndef KINIT_H
#define KINIT_H
#include <mach/mach_init.h>
#include <util/str.h>
#include <stdarg.h>

#define kinit_to_msg(buf, msg, ...) sprintf(buf, msg, __VA_ARGS__)
#define kinit_warn(buf, msg, ...) __kinit_warn(kinit_to_msg(buf, msg, __VA_ARGS__), \
    __FILE__, __func__, __LINE__)
#define kinit_panic(buf, msg, ...) __kinit_panic(kinit_to_msg(buf, msg, __VA_ARGS__), \
    __FILE__, __func__, __LINE__)
#define kinit_info(buf, msg, ...) __kinit_info(kinit_to_msg(buf, msg, __VA_ARGS__), \
    __FILE__, __func__, __LINE__)

inline void __kinit_warn(const char *msg, const char *file, const char *func, int line) {
    mach_init_printf("[KINIT_WARN][%s:%i:%s] %s\n", file, line, func, msg);
}

inline void __kinit_info(const char *msg, const char *file, const char *func, int line) {
    mach_init_printf("[KINIT_INFO][%s:%i:%s] %s\n", file, line, func, msg);
}

inline void __kinit_panic(const char *msg, const char *file, const char *func, int line) {
    mach_init_printf("[KINIT_PANIC][%s:%i:%s] %s\n", file, line, func, msg);
	
    /* dump stack here? */
	
    while(1);
}

/* 
 * expectation that mmu is set up higher half with
 * continuous mmu page tables. 
 * 
 * kernel_init expects that both the hmi & kernel regions
 * are mapped into memory
 */

/* kernel_init.c */
// kernel_init(mach, atags, k_stack, mmu_pgd, mmu_pgtb)
void kernel_init(void);



#endif
