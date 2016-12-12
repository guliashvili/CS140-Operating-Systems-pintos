#ifndef USERPROG_EXCEPTION_H
#define USERPROG_EXCEPTION_H
#include <stdbool.h>
#include <stdint.h>
/* Page fault error code bits that describe the cause of the exception.  */
#define PF_P 0x1    /* 0: not-present page. 1: access rights violation. */
#define PF_W 0x2    /* 0: read, 1: write. */
#define PF_U 0x4    /* 0: kernel, 1: user process. */

void exception_init (void);
void exception_print_stats (void);
bool stack_resized(uint32_t esp, void *p);
#endif /* userprog/exception.h */
