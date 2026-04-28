/*
 * @Author: 野生指针 <1703214369@qq.com>
 * @Date: 2026-04-21 23:32:40
 * @LastEditors: 野生指针 <1703214369@qq.com>
 * @LastEditTime: 2026-04-28 23:39:44
 * @FilePath: /inlinehooktest/bypass.c
 * @Description: 联系我QQ 1703214369
 * 
 * Copyright (c) 2026 by 野生指针 <1703214369@qq.com>, All Rights Reserved. 
 */
#include "bypass.h"
#include "arch_helper.h"
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/list.h>

#define INSTRUCTION_SIZE 4

int bypass_cfi(void)
{
    int ret = 0;
    unsigned int RET = 0xD65F03C0;
    unsigned int MOV_X0_1 = 0xD2800020;
    unsigned long f__cfi_slowpath;
    unsigned long f__cfi_slowpath_diag;
    unsigned long f_cfi_slowpath;
    unsigned long f__cfi_check_fail;
    unsigned long f__ubsan_handle_cfi_check_fail_abort;
    unsigned long f__ubsan_handle_cfi_check_fail;
    unsigned long freport_cfi_failure;

    f__cfi_slowpath = kallsyms_lookup_name_ex("__cfi_slowpath");
    if (f__cfi_slowpath) {
        unsigned int *p = (unsigned int *)f__cfi_slowpath;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_info("bypass: patched __cfi_slowpath\n");
        } else {
            pr_info("bypass: __cfi_slowpath already patched\n");
        }
    }

    f__cfi_slowpath_diag = kallsyms_lookup_name_ex("__cfi_slowpath_diag");
    if (f__cfi_slowpath_diag) {
        unsigned int *p = (unsigned int *)f__cfi_slowpath_diag;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_info("bypass: patched __cfi_slowpath_diag\n");
        } else {
            pr_info("bypass: __cfi_slowpath_diag already patched\n");
        }
    }

    f_cfi_slowpath = kallsyms_lookup_name_ex("_cfi_slowpath");
    if (f_cfi_slowpath) {
        unsigned int *p = (unsigned int *)f_cfi_slowpath;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_info("bypass: patched _cfi_slowpath\n");
        } else {
            pr_info("bypass: _cfi_slowpath already patched\n");
        }
    }

    f__cfi_check_fail = kallsyms_lookup_name_ex("__cfi_check_fail");
    if (f__cfi_check_fail) {
        unsigned int *p = (unsigned int *)f__cfi_check_fail;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_info("bypass: patched __cfi_check_fail\n");
        } else {
            pr_info("bypass: __cfi_check_fail already patched\n");
        }
    }

    f__ubsan_handle_cfi_check_fail_abort = kallsyms_lookup_name_ex("__ubsan_handle_cfi_check_fail_abort");
    if (f__ubsan_handle_cfi_check_fail_abort) {
        unsigned int *p = (unsigned int *)f__ubsan_handle_cfi_check_fail_abort;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_info("bypass: patched __ubsan_handle_cfi_check_fail_abort\n");
        } else {
            pr_info("bypass: __ubsan_handle_cfi_check_fail_abort already patched\n");
        }
    }

    f__ubsan_handle_cfi_check_fail = kallsyms_lookup_name_ex("__ubsan_handle_cfi_check_fail");
    if (f__ubsan_handle_cfi_check_fail) {
        unsigned int *p = (unsigned int *)f__ubsan_handle_cfi_check_fail;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_info("bypass: patched __ubsan_handle_cfi_check_fail\n");
        } else {
            pr_info("bypass: __ubsan_handle_cfi_check_fail already patched\n");
        }
    }

    freport_cfi_failure = kallsyms_lookup_name_ex("report_cfi_failure");
    if (freport_cfi_failure) {
        unsigned int *p = (unsigned int *)freport_cfi_failure;

        if (*p != MOV_X0_1) {
            hook_write_range(p, &MOV_X0_1, INSTRUCTION_SIZE);
            hook_write_range(p + 1, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_info("bypass: patched report_cfi_failure\n");
        } else {
            pr_info("bypass: report_cfi_failure already patched\n");
        }
    }

    pr_info("bypass: CFI bypass completed, patched %d functions\n", ret);
    return ret;
}

int disable_kprobe_blacklist(void)
{
    struct kprobe_blacklist_entry *ent;
    struct list_head *kprobe_blacklist;
    int count = 0;

    kprobe_blacklist = (struct list_head *)kallsyms_lookup_name_ex("kprobe_blacklist");
    if (!kprobe_blacklist) {
        pr_err("bypass: kprobe_blacklist not found\n");
        return -ENOENT;
    }

    list_for_each_entry(ent, kprobe_blacklist, list) {
        if (!ent || ent->start_addr == 0 || ent->end_addr == 0)
            continue;
        count++;
        ent->start_addr = 0;
        ent->end_addr = 0;
    }

    pr_info("bypass: disabled %d kprobe blacklist entries\n", count);
    return 0;
}
