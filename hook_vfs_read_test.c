/*
 * @Author: 野生指针 <1703214369@qq.com>
 * @Date: 2026-04-22 14:23:05
 * @LastEditors: 野生指针 <1703214369@qq.com>
 * @LastEditTime: 2026-04-28 23:38:26
 * @FilePath: /inlinehooktest/hook_vfs_read_test.c
 * @Description: 联系我QQ 1703214369
 * 
 * Copyright (c) 2026 by 野生指针 <1703214369@qq.com>, All Rights Reserved. 
 */
#include "hook_vfs_read_test.h"
#include "inline_hook.h"
#include "arch_helper.h"
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/path.h>
#include <linux/dcache.h>

HOOK_FUNC_TEMPLATE(vfs_read);
static __nocfi __attribute__((used)) ssize_t hook_vfs_read(
    struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    ssize_t (*origin_vfs_read)(struct file *, char __user *, size_t, loff_t *);
    char *path_buf;
    char *path;

    path_buf = kmalloc(256, GFP_KERNEL);
    if (path_buf) {
        path = d_path(&file->f_path, path_buf, 256);
        if (!IS_ERR(path)) {
            if (!strnstr(path, "/dev/kmsg", 256 - (path - path_buf)) &&
                !strnstr(path, "/proc/kmsg", 256 - (path - path_buf)) &&
                !strnstr(path, "[timerfd]", 256 - (path - path_buf)) &&
                !strnstr(path, "/run/log", 256 - (path - path_buf)) &&
                !strnstr(path, "/var/log", 256 - (path - path_buf)))
                pr_info("vfs_read_hook: reading %s (count=%zu)\n", path, count);
        }
        kfree(path_buf);
    }

    origin_vfs_read = (ssize_t (*)(struct file *, char __user *, size_t, loff_t *))
        GET_CODESPACE_ADDERSS(vfs_read);
    return origin_vfs_read(file, buf, count, pos);
}

static void *vfs_read_fn;

int hook_vfs_read_init(void)
{
    vfs_read_fn = (void *)kallsyms_lookup_name_ex("vfs_read");
    if (!vfs_read_fn) {
        pr_err("hook_vfs_read: vfs_read not found\n");
        return -ENOENT;
    }

    if (HIJACK_TARGET_PREP_HOOK(vfs_read_fn, vfs_read)) {
        pr_err("hook_vfs_read: prepare failed\n");
        return -EFAULT;
    }

    if (hijack_target_enable(vfs_read_fn)) {
        pr_err("hook_vfs_read: enable failed\n");
        hijack_target_disable(vfs_read_fn, true);
        return -EFAULT;
    }

    pr_info("hook_vfs_read: vfs_read hooked at %p\n", vfs_read_fn);
    return 0;
}

void hook_vfs_read_exit(void)
{
    if (vfs_read_fn)
        hijack_target_disable(vfs_read_fn, true);
    pr_info("hook_vfs_read: unhooked\n");
}
