#ifndef ARCH_HELPER_H
#define ARCH_HELPER_H

#include <linux/types.h>

/* 架构相关辅助函数 */

/* 通过名称查找内核符号地址 */
unsigned long kallsyms_lookup_name_ex(const char *name);

/* 向只读内存写入指令 */
int hook_write_range(void *target, void *source, int size);

/* 初始化架构相关函数 */
int arch_helper_init(void);

#endif
