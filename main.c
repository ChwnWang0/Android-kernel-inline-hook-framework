#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kmsg_dump.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/fs.h>

#include "kp_hook.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PWY_Fixed");
MODULE_DESCRIPTION("KernelPatch inline hook runtime port demo");
MODULE_VERSION("2.0");

static __attribute__((naked)) int add(int a, int b)
{
    asm volatile(
        "add w0, w0, w1\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "ret\n"
    );
}

static bool panic_store_ready;
static bool kp_hook_ready;
static bool add_hook_installed;

HOOK_FUNC_TEMPLATE(add);
static __nocfi __attribute__((used)) int hook_add(int a, int b)
{
    int (*origin_add)(int, int);

    pr_info("inlinehooktest: hook_add called with %d + %d\n", a, b);

    origin_add = (int (*)(int, int))GET_CODESPACE_ADDERSS(add);
    return origin_add(a, b);
}

static bool is_panic;
static struct file *(*filp_open_ptr)(const char *, int, umode_t);
static int (*filp_close_ptr)(struct file *, fl_owner_t);
static ssize_t (*kernel_write_ptr)(struct file *, const void *, size_t, loff_t *);

static int (*aarch64_insn_write_ptr)(void *, u32);
static void (*flush_icache_range_ptr)(unsigned long, unsigned long);
typedef unsigned long (*main_kallsyms_lookup_name_t)(const char *);

#ifdef CONFIG_CFI_CLANG
#define NO_CFI __nocfi
#else
#define NO_CFI
#endif

static unsigned long NO_CFI call_kln(main_kallsyms_lookup_name_t f, const char *n)
{
    return f(n);
}

static unsigned long kallsyms_lookup_name_ex(const char *name)
{
    static main_kallsyms_lookup_name_t lookup_name;

    if (lookup_name == NULL) {
        struct kprobe kp = { .symbol_name = "kallsyms_lookup_name" };

        if (register_kprobe(&kp) < 0)
            return 0;

        lookup_name = (main_kallsyms_lookup_name_t)kp.addr;
        unregister_kprobe(&kp);

        if (lookup_name == NULL) {
            pr_err("inlinehooktest: kallsyms_lookup_name not found\n");
            return 0;
        }

        pr_info("inlinehooktest: kallsyms_lookup_name_ex found at %p\n", lookup_name);
    }

    return call_kln(lookup_name, name);
}

static int init_arch(void)
{
    aarch64_insn_write_ptr = (void *)kallsyms_lookup_name_ex("aarch64_insn_write");
    flush_icache_range_ptr = (void *)kallsyms_lookup_name_ex("caches_clean_inval_pou");

    if (!flush_icache_range_ptr)
        flush_icache_range_ptr = (void *)kallsyms_lookup_name_ex("__flush_icache_range");

    return !(aarch64_insn_write_ptr && flush_icache_range_ptr);
}

static __nocfi int hook_write_range(void *target, void *source, int size)
{
    int ret = 0;
    int i;
    char *dst = target;
    char *src = source;

    for (i = 0; i < size; i += INSTRUCTION_SIZE) {
        ret = aarch64_insn_write_ptr(dst + i, *(u32 *)(src + i));
        if (ret)
            goto out;
    }
    flush_icache_range_ptr((unsigned long)target, (unsigned long)target + size);

out:
    return ret;
}

static int cfi_bypass(void)
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
            pr_err("inlinehooktest: patch __cfi_slowpath successed\n");
        } else {
            pr_info("inlinehooktest: __cfi_slowpath already patched\n");
        }
    }

    f__cfi_slowpath_diag = kallsyms_lookup_name_ex("__cfi_slowpath_diag");
    if (f__cfi_slowpath_diag) {
        unsigned int *p = (unsigned int *)f__cfi_slowpath_diag;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_err("inlinehooktest: patch __cfi_slowpath_diag successed\n");
        } else {
            pr_info("inlinehooktest: __cfi_slowpath_diag already patched\n");
        }
    }

    f_cfi_slowpath = kallsyms_lookup_name_ex("_cfi_slowpath");
    if (f_cfi_slowpath) {
        unsigned int *p = (unsigned int *)f_cfi_slowpath;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_err("inlinehooktest: patch _cfi_slowpath successed\n");
        } else {
            pr_info("inlinehooktest: _cfi_slowpath already patched\n");
        }
    }

    f__cfi_check_fail = kallsyms_lookup_name_ex("__cfi_check_fail");
    if (f__cfi_check_fail) {
        unsigned int *p = (unsigned int *)f__cfi_check_fail;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_err("inlinehooktest: patch __cfi_check_fail successed\n");
        } else {
            pr_info("inlinehooktest: __cfi_check_fail already patched\n");
        }
    }

    f__ubsan_handle_cfi_check_fail_abort = kallsyms_lookup_name_ex("__ubsan_handle_cfi_check_fail_abort");
    if (f__ubsan_handle_cfi_check_fail_abort) {
        unsigned int *p = (unsigned int *)f__ubsan_handle_cfi_check_fail_abort;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_err("inlinehooktest: patch __ubsan_handle_cfi_check_fail_abort successed\n");
        } else {
            pr_info("inlinehooktest: __ubsan_handle_cfi_check_fail_abort already patched\n");
        }
    }

    f__ubsan_handle_cfi_check_fail = kallsyms_lookup_name_ex("__ubsan_handle_cfi_check_fail");
    if (f__ubsan_handle_cfi_check_fail) {
        unsigned int *p = (unsigned int *)f__ubsan_handle_cfi_check_fail;

        if (*p != RET) {
            hook_write_range(p, &RET, INSTRUCTION_SIZE);
            ret++;
            pr_err("inlinehooktest: patch __ubsan_handle_cfi_check_fail successed\n");
        } else {
            pr_info("inlinehooktest: __ubsan_handle_cfi_check_fail already patched\n");
        }
    }

    freport_cfi_failure = kallsyms_lookup_name_ex("report_cfi_failure");
    if (freport_cfi_failure) {
        unsigned int *p = (unsigned int *)freport_cfi_failure;

        if (*p != MOV_X0_1) {
            hook_write_range(p, &MOV_X0_1, INSTRUCTION_SIZE);
            hook_write_range(p + 1, &RET, INSTRUCTION_SIZE);
            ret++;
        } else {
            pr_info("inlinehooktest: report_cfi_failure already patched\n");
        }
    }

    return ret;
}

static __nocfi int ret_handler_dump_backtrace(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct file *file;
    struct kmsg_dumper dumper = { .active = true };
    size_t len = 0;
    char *line_buf;
    ssize_t ret;
    loff_t pos = 0;

    if (!is_panic)
        return 0;

    line_buf = kmalloc(512, GFP_ATOMIC);
    if (!line_buf)
        return 0;

    file = filp_open_ptr("/data/panic_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(file)) {
        kfree(line_buf);
        return 0;
    }

    kmsg_dump_rewind(&dumper);
    while (kmsg_dump_get_line(&dumper, false, line_buf, 512, &len)) {
        ret = kernel_write_ptr(file, line_buf, len, &pos);
        if (ret < 0)
            break;
    }

    vfs_fsync(file, 0);
    filp_close_ptr(file, NULL);
    kfree(line_buf);
    is_panic = false;
    return 0;
}

static struct kretprobe kp_dump_backtrace = {
    .handler = ret_handler_dump_backtrace,
    .maxactive = 20,
    .kp.symbol_name = "dump_backtrace",
};

static int handler_panic_pre(struct kprobe *p, struct pt_regs *regs)
{
    is_panic = true;
    return 0;
}

static struct kprobe kp_panic = {
    .symbol_name = "panic",
    .pre_handler = handler_panic_pre,
};

static struct kprobe kp_die = {
    .symbol_name = "die",
    .pre_handler = handler_panic_pre,
};

static int panic_store_register(void)
{
    filp_open_ptr = (void *)kallsyms_lookup_name_ex("filp_open");
    filp_close_ptr = (void *)kallsyms_lookup_name_ex("filp_close");
    kernel_write_ptr = (void *)kallsyms_lookup_name_ex("kernel_write");

    if (!filp_open_ptr || !kernel_write_ptr) {
        pr_err("inlinehooktest: panic store critical symbols not found\n");
        return -ENXIO;
    }

    if (register_kretprobe(&kp_dump_backtrace) < 0)
        return -EINVAL;

    if (register_kprobe(&kp_panic) < 0) {
        unregister_kretprobe(&kp_dump_backtrace);
        return -EINVAL;
    }

    if (register_kprobe(&kp_die) < 0) {
        unregister_kretprobe(&kp_dump_backtrace);
        unregister_kprobe(&kp_panic);
        return -EINVAL;
    }

    pr_info("inlinehooktest: panic store probes registered\n");
    panic_store_ready = true;
    return 0;
}

static void panic_store_unregister(void)
{
    if (!panic_store_ready)
        return;

    unregister_kretprobe(&kp_dump_backtrace);
    unregister_kprobe(&kp_panic);
    unregister_kprobe(&kp_die);
    panic_store_ready = false;
    pr_info("inlinehooktest: panic store probes unregistered\n");
}

static int disable_kprobe_blacklist(void)
{
    struct kprobe_blacklist_entry *ent;
    struct list_head *kprobe_blacklist;
    int count = 0;

    kprobe_blacklist = (struct list_head *)kallsyms_lookup_name_ex("kprobe_blacklist");
    if (!kprobe_blacklist) {
        pr_err("kprobe_blacklist not found\n");
        return -ENOENT;
    }

    list_for_each_entry(ent, kprobe_blacklist, list) {
        if (!ent || ent->start_addr == 0 || ent->end_addr == 0)
            continue;
        count++;
        ent->start_addr = 0;
        ent->end_addr = 0;
    }

    pr_info("Disabled %d kprobe blacklist entries\n", count);
    return 0;
}

static int __init inline_hook_demo_init(void)
{
    int ret;

    ret = panic_store_register();
    if (ret) {
        pr_err("inlinehooktest: panic_store_register failed: %d\n", ret);
        return ret;
    }

    ret = disable_kprobe_blacklist();
    if (ret) {
        pr_err("inlinehooktest: disable_kprobe_blacklist failed: %d\n", ret);
        return ret;
    }

    ret = init_arch();
    if (ret) {
        pr_err("inlinehooktest: init_arch failed: %d\n", ret);
        panic_store_unregister();
        return ret;
    }

    pr_info("inlinehooktest: NO_CFI is enabled, patched: %d\n", cfi_bypass());

    ret = kp_hook_runtime_init();
    if (ret) {
        pr_err("inlinehooktest: kp_hook runtime init failed: %d\n", ret);
        panic_store_unregister();
        return ret;
    }
    kp_hook_ready = true;

    ret = add(20, 10);
    pr_info("inlinehooktest: before hook 20 + 10 = %d\n", ret);

    ret = HIJACK_TARGET_PREP_HOOK((void *)add, add);
    if (ret) {
        pr_err("inlinehooktest: hijack_target_prepare failed: %d\n", ret);
        kp_hook_runtime_exit();
        kp_hook_ready = false;
        panic_store_unregister();
        return -EINVAL;
    }

    ret = hijack_target_enable((void *)add);
    if (ret) {
        pr_err("inlinehooktest: hijack_target_enable failed: %d\n", ret);
        kp_hook_runtime_exit();
        kp_hook_ready = false;
        panic_store_unregister();
        return -EINVAL;
    }
    add_hook_installed = true;

    ret = add(20, 10);
    pr_info("inlinehooktest: after hook 20 + 10 = %d\n", ret);

    return 0;
}

static void __exit inline_hook_demo_exit(void)
{
    int ret;

    if (add_hook_installed) {
        hijack_target_disable((void *)add, true);
        add_hook_installed = false;
    }

    panic_store_unregister();
    ret = add(20, 10);
    pr_info("inlinehooktest: after unhook 20 + 10 = %d\n", ret);

    if (kp_hook_ready) {
        kp_hook_runtime_exit();
        kp_hook_ready = false;
    }
}

module_init(inline_hook_demo_init);
module_exit(inline_hook_demo_exit);

#include "kp_hook.c"
