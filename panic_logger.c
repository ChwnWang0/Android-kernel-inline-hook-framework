#include "panic_logger.h"
#include "arch_helper.h"
#include <linux/kernel.h>
#include <linux/kmsg_dump.h>
#include <linux/kprobes.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

static bool is_panic;
static bool panic_logger_ready;
static struct file *(*filp_open_ptr)(const char *, int, umode_t);
static int (*filp_close_ptr)(struct file *, fl_owner_t);
static ssize_t (*kernel_write_ptr)(struct file *, const void *, size_t, loff_t *);

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

int panic_logger_init(void)
{
    filp_open_ptr = (void *)kallsyms_lookup_name_ex("filp_open");
    filp_close_ptr = (void *)kallsyms_lookup_name_ex("filp_close");
    kernel_write_ptr = (void *)kallsyms_lookup_name_ex("kernel_write");

    if (!filp_open_ptr || !kernel_write_ptr) {
        pr_err("panic_logger: critical symbols not found\n");
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

    pr_info("panic_logger: initialized\n");
    panic_logger_ready = true;
    return 0;
}

void panic_logger_exit(void)
{
    if (!panic_logger_ready)
        return;

    unregister_kretprobe(&kp_dump_backtrace);
    unregister_kprobe(&kp_panic);
    unregister_kprobe(&kp_die);
    panic_logger_ready = false;
    pr_info("panic_logger: exited\n");
}
