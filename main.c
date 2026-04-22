#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "inline_hook.h"
#include "arch_helper.h"
#include "bypass.h"
#include "panic_logger.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PWY");
MODULE_DESCRIPTION("ARM64 inline hook framework with instruction relocation");
MODULE_VERSION("3.0");

/* ---- Test target function ---- */

static noinline int add(int a, int b)
{
    int result;
    if (a > 100) {
        pr_info("add: a is large: %d\n", a);
        result = a + b + 1;
    } else {
        pr_info("add: a is normal: %d\n", a);
        result = a + b;
    }
    return result;
}

/* ---- Hook function ---- */

static bool hook_ready;
static bool add_hook_installed;

HOOK_FUNC_TEMPLATE(add);
static __nocfi __attribute__((used)) int hook_add(int a, int b)
{
    int (*origin_add)(int, int);

    pr_info("hook_add: intercepted %d + %d\n", a, b);

    origin_add = (int (*)(int, int))GET_CODESPACE_ADDERSS(add);
    return origin_add(a, b);
}

/* ---- Module init/exit ---- */

static int __init inline_hook_demo_init(void)
{
    int ret;

    ret = panic_logger_init();
    if (ret) {
        pr_err("main: panic_logger_init failed: %d\n", ret);
        return ret;
    }

    ret = disable_kprobe_blacklist();
    if (ret)
        pr_warn("main: disable_kprobe_blacklist failed: %d (non-fatal)\n", ret);

    ret = arch_helper_init();
    if (ret) {
        pr_err("main: arch_helper_init failed: %d\n", ret);
        panic_logger_exit();
        return ret;
    }

    pr_info("main: CFI bypass patched %d functions\n", bypass_cfi());

    ret = inline_hook_init();
    if (ret) {
        pr_err("main: inline_hook_init failed: %d\n", ret);
        panic_logger_exit();
        return ret;
    }
    hook_ready = true;

    /* Test: hook the add function */
    ret = add(20, 10);
    pr_info("main: before hook 20 + 10 = %d\n", ret);

    ret = HIJACK_TARGET_PREP_HOOK((void *)add, add);
    if (ret) {
        pr_err("main: hijack_target_prepare failed: %d\n", ret);
        goto fail_hook;
    }

    ret = hijack_target_enable((void *)add);
    if (ret) {
        pr_err("main: hijack_target_enable failed: %d\n", ret);
        goto fail_hook;
    }
    add_hook_installed = true;

    ret = add(20, 10);
    pr_info("main: after hook 20 + 10 = %d\n", ret);

    return 0;

fail_hook:
    inline_hook_exit();
    hook_ready = false;
    panic_logger_exit();
    return -EINVAL;
}

static void __exit inline_hook_demo_exit(void)
{
    int ret;

    if (add_hook_installed) {
        hijack_target_disable((void *)add, true);
        add_hook_installed = false;
    }

    ret = add(20, 10);
    pr_info("main: after unhook 20 + 10 = %d\n", ret);

    if (hook_ready) {
        inline_hook_exit();
        hook_ready = false;
    }

    panic_logger_exit();
}

module_init(inline_hook_demo_init);
module_exit(inline_hook_demo_exit);

#include "arch_helper.c"
#include "bypass.c"
#include "panic_logger.c"
#include "inline_hook.c"
