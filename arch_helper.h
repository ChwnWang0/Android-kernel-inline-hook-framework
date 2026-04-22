#ifndef ARCH_HELPER_H
#define ARCH_HELPER_H

#include <linux/types.h>

/* Architecture-specific helper functions */

/* Lookup kernel symbol by name */
unsigned long kallsyms_lookup_name_ex(const char *name);

/* Write instruction to read-only memory */
int hook_write_range(void *target, void *source, int size);

/* Initialize architecture-specific functions */
int arch_helper_init(void);

#endif
