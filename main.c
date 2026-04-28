#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "inline_hook.h"
#include "arch_helper.h"
#include "bypass.h"
#include "hook_vfs_read_test.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PWY");
MODULE_DESCRIPTION("ARM64 inline hook framework with instruction relocation");
MODULE_VERSION("3.0");

/* ---- 测试目标函数 ---- */

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

/* ---- Hook 函数 ---- */

static bool hook_ready;
static bool add_hook_installed;
static bool vfs_read_hook_installed;

HOOK_FUNC_TEMPLATE(add);
static __nocfi __attribute__((used)) int hook_add(int a, int b)
{
    int (*origin_add)(int, int);

    pr_info("hook_add: intercepted %d + %d\n", a, b);

    origin_add = (int (*)(int, int))GET_CODESPACE_ADDERSS(add);
    return origin_add(a, b);
}

/* ---- 模块初始化/退出 ---- */

static int __init inline_hook_demo_init(void)
{
    int ret;

    ret = disable_kprobe_blacklist();
    if (ret)
        pr_warn("main: disable_kprobe_blacklist failed: %d (non-fatal)\n", ret);

    ret = arch_helper_init();
    if (ret) {
        pr_err("main: arch_helper_init failed: %d\n", ret);
        return ret;
    }

    pr_info("main: CFI bypass patched %d functions\n", bypass_cfi());

    ret = inline_hook_init();
    if (ret) {
        pr_err("main: inline_hook_init failed: %d\n", ret);
        return ret;
    }
    hook_ready = true;

    /* 测试：hook add 函数 */
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

    /* Hook vfs_read 函数 */
    ret = hook_vfs_read_init();
    if (ret)
        pr_warn("main: hook_vfs_read_init failed: %d (non-fatal)\n", ret);
    else
        vfs_read_hook_installed = true;

    return 0;

fail_hook:
    inline_hook_exit();
    hook_ready = false;
    return -EINVAL;
}

static void __exit inline_hook_demo_exit(void)
{
    int ret;

    if (vfs_read_hook_installed) {
        hook_vfs_read_exit();
        vfs_read_hook_installed = false;
    }

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
}

module_init(inline_hook_demo_init);
module_exit(inline_hook_demo_exit);
