/*
 * @Author: 野生指针 <1703214369@qq.com>
 * @Date: 2026-04-22 12:35:10
 * @LastEditors: 野生指针 <1703214369@qq.com>
 * @LastEditTime: 2026-04-28 23:40:01
 * @FilePath: /inlinehooktest/arch_helper.h
 * @Description: 联系我QQ 1703214369
 * 
 * Copyright (c) 2026 by 野生指针 <1703214369@qq.com>, All Rights Reserved. 
 */
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
