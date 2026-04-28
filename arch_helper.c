/*
 * @Author: 野生指针 <1703214369@qq.com>
 * @Date: 2026-04-22 12:35:10
 * @LastEditors: 野生指针 <1703214369@qq.com>
 * @LastEditTime: 2026-04-28 23:40:26
 * @FilePath: /inlinehooktest/arch_helper.c
 * @Description: 联系我QQ 1703214369
 * 
 * Copyright (c) 2026 by 野生指针 <1703214369@qq.com>, All Rights Reserved. 
 */
#include "arch_helper.h"
#include <linux/kprobes.h>
#include <linux/kernel.h>

#ifdef CONFIG_CFI_CLANG
#define NO_CFI __nocfi
#else
#define NO_CFI
#endif

static int (*aarch64_insn_write_ptr)(void *, u32);
static void (*flush_icache_range_ptr)(unsigned long, unsigned long);
typedef unsigned long (*main_kallsyms_lookup_name_t)(const char *);
static main_kallsyms_lookup_name_t lookup_name;

static unsigned long NO_CFI call_kln(main_kallsyms_lookup_name_t f, const char *n)
{
    return f(n);
}

unsigned long kallsyms_lookup_name_ex(const char *name)
{
    if (lookup_name == NULL) {
        struct kprobe kp = { .symbol_name = "kallsyms_lookup_name" };

        if (register_kprobe(&kp) < 0)
            return 0;

        lookup_name = (main_kallsyms_lookup_name_t)kp.addr;
        unregister_kprobe(&kp);

        if (lookup_name == NULL) {
            pr_err("arch_helper: kallsyms_lookup_name not found\n");
            return 0;
        }

        pr_info("arch_helper: kallsyms_lookup_name found at %p\n", lookup_name);
    }

    return call_kln(lookup_name, name);
}

int NO_CFI hook_write_range(void *target, void *source, int size)
{
    int ret = 0;
    int i;
    char *dst = target;
    char *src = source;

    for (i = 0; i < size; i += 4) {
        ret = aarch64_insn_write_ptr(dst + i, *(u32 *)(src + i));
        if (ret)
            goto out;
    }
    flush_icache_range_ptr((unsigned long)target, (unsigned long)target + size);

out:
    return ret;
}

int arch_helper_init(void)
{
    aarch64_insn_write_ptr = (void *)kallsyms_lookup_name_ex("aarch64_insn_write");
    flush_icache_range_ptr = (void *)kallsyms_lookup_name_ex("caches_clean_inval_pou");

    if (!flush_icache_range_ptr)
        flush_icache_range_ptr = (void *)kallsyms_lookup_name_ex("__flush_icache_range");

    if (!aarch64_insn_write_ptr || !flush_icache_range_ptr) {
        pr_err("arch_helper: failed to find required symbols\n");
        return -ENOENT;
    }

    pr_info("arch_helper: initialized\n");
    return 0;
}
