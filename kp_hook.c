#include "kp_hook.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/jhash.h>
#include <linux/rwsem.h>
#include <linux/stop_machine.h>
#include <linux/delay.h>
#include <linux/stacktrace.h>
#include <linux/kallsyms.h>
#include <asm/cacheflush.h>

static const char long_jmp_code[24] =
    "\xe1\x03\xbe\xa9"
    "\x40\x00\x00\x58"
    "\x00\x00\x5f\xd6"
    "\x00\x00\x00\x00"
    "\x00\x00\x00\x00"
    "\xe1\x03\xc2\xa8";

static DEFINE_HASHTABLE(all_hijack_targets, DEFAULT_HASH_BUCKET_BITS);
static DECLARE_RWSEM(hijack_targets_hashtable_lock);

static int (*kallsyms_lookup_size_offset_ptr)(unsigned long,
    unsigned long *, unsigned long *);

#define MAX_STACK_TRACE_DEPTH 64
static unsigned int (*stack_trace_save_tsk_ptr)(struct task_struct *,
    unsigned long *, unsigned int, unsigned int);

static int (*stop_machine_ptr)(int (*)(void *), void *, const struct cpumask *);

static bool g_runtime_ready;

static inline void fill_long_jmp(void *fill_dest, void *hijack_to_func)
{
    memcpy(fill_dest, long_jmp_code, sizeof(long_jmp_code));
    memcpy(fill_dest + 3 * INSTRUCTION_SIZE, &hijack_to_func, sizeof(void *));
}

static bool check_instruction_can_hijack(uint32_t instruction)
{
    switch (instruction & 0x9f000000u) {
    case 0x10000000u:
    case 0x90000000u:
        return false;
    }
    switch (instruction & 0xfc000000u) {
    case 0x14000000u:
    case 0x94000000u:
        return false;
    }
    switch (instruction & 0xff000000u) {
    case 0x54000000u:
        return false;
    }
    switch (instruction & 0x7e000000u) {
    case 0x34000000u:
    case 0x36000000u:
        return false;
    }
    switch (instruction & 0xbf000000u) {
    case 0x18000000u:
        return false;
    }
    switch (instruction & 0x3f000000u) {
    case 0x1c000000u:
        return false;
    }
    switch (instruction & 0xff000000u) {
    case 0x98000000u:
        return false;
    }
    return true;
}

static void *follow_trampoline(void *target)
{
    uint32_t first_inst = *(uint32_t *)target;
    uint32_t second_inst = *(uint32_t *)(target + INSTRUCTION_SIZE);

    if (first_inst == 0xd503245f && (second_inst & 0xfc000000) == 0x14000000) {
        /* This is a trampoline: bti c + b offset */
        int32_t offset = (second_inst & 0x03ffffff) << 2;
        /* Sign extend from 28 bits to 32 bits */
        if (offset & 0x08000000)
            offset |= 0xf0000000;
        target = target + INSTRUCTION_SIZE + offset;
        pr_info("kp_hook: followed trampoline to %p\n", target);
    }
    return target;
}

static bool check_target_can_hijack(void *target)
{
    int offset;

    pr_info("kp_hook: checking target at %p\n", target);
    for (offset = 0; offset < HOOK_TARGET_OFFSET + HIJACK_SIZE; offset += INSTRUCTION_SIZE) {
        uint32_t inst = *(uint32_t *)(target + offset);
        pr_info("kp_hook: [%d] %08x\n", offset, inst);
        if (!check_instruction_can_hijack(inst)) {
            pr_err("kp_hook: instruction %x at offset %d cannot be hijacked\n",
                inst, offset);
            return false;
        }
    }
    return true;
}

static __nocfi bool check_function_length_enough(void *target)
{
    unsigned long symbolsize = 0, offset = 0;
    unsigned long pos;

    if (!kallsyms_lookup_size_offset_ptr)
        return true;

    pos = kallsyms_lookup_size_offset_ptr((unsigned long)target, &symbolsize, &offset);

    /* If symbol not found or no size info, allow the hook */
    if (!pos || symbolsize == 0)
        return true;

    /* If symbolsize is suspiciously small (< 16 bytes), it's likely inaccurate
     * for module-local static functions. Allow the hook. */
    if (symbolsize < 16)
        return true;

    /* Only reject if we have reliable size info AND it's definitely too short */
    if (symbolsize < HIJACK_SIZE + offset) {
        pr_warn("kp_hook: symbol size %lu offset %lu too short for hijack\n",
            symbolsize, offset);
        return false;
    }

    return true;
}

static int fill_hook_template_code_space(void *hook_template_code_space,
    void *target_code, void *return_addr)
{
    unsigned char tmp_code[HIJACK_SIZE * 2];

    memset(tmp_code, 0, sizeof(tmp_code));
    memcpy(tmp_code, target_code, HIJACK_SIZE);
    fill_long_jmp(tmp_code + HIJACK_SIZE, return_addr);
    return hook_write_range(hook_template_code_space, tmp_code, sizeof(tmp_code));
}

struct do_hijack_struct {
    void *dest;
    void *source;
    void *hook_func;
};

static __nocfi int stack_activeness_safety_check(unsigned long addr,
    unsigned long hook_func)
{
    struct task_struct *g, *t;
    unsigned long entries[MAX_STACK_TRACE_DEPTH];
    unsigned int nr_entries;
    int ret = 0;
    int i;

    if (!stack_trace_save_tsk_ptr)
        return 0;

    for_each_process_thread(g, t) {
        nr_entries = stack_trace_save_tsk_ptr(t, entries,
            MAX_STACK_TRACE_DEPTH, 0);
        for (i = 0; i < nr_entries; i++) {
            if (entries[i] >= addr && entries[i] < addr + HIJACK_SIZE) {
                pr_warn("kp_hook: stack conflict PID %d Comm %.20s [<%lx>]\n",
                    t->pid, t->comm, entries[i]);
                return -2;
            }
            if (hook_func && kallsyms_lookup_size_offset_ptr) {
                unsigned long symbolsize = 0, offset = 0;
                if (kallsyms_lookup_size_offset_ptr(hook_func, &symbolsize, &offset)) {
                    if (entries[i] >= hook_func - offset &&
                        entries[i] < hook_func - offset + symbolsize) {
                        pr_warn("kp_hook: hook_func conflict PID %d Comm %.20s\n",
                            t->pid, t->comm);
                        ret = -1;
                    }
                }
            }
        }
    }
    return ret;
}

static __nocfi int do_hijack_target(void *data)
{
    struct do_hijack_struct *dhs = data;
    int ret;

    if (stack_trace_save_tsk_ptr) {
        ret = stack_activeness_safety_check((unsigned long)dhs->dest,
            (unsigned long)dhs->hook_func);
        if (ret == -2)
            return ret;
    } else {
        ret = 0;
    }

    if (hook_write_range(dhs->dest, dhs->source, HIJACK_SIZE))
        return -3;

    return ret;
}

int kp_hook_runtime_init(void)
{
    if (g_runtime_ready)
        return 0;

    hash_init(all_hijack_targets);

    kallsyms_lookup_size_offset_ptr = (void *)kallsyms_lookup_name_ex(
        "kallsyms_lookup_size_offset");
    stack_trace_save_tsk_ptr = (void *)kallsyms_lookup_name_ex(
        "stack_trace_save_tsk");
    stop_machine_ptr = (void *)kallsyms_lookup_name_ex("stop_machine");

    if (!stop_machine_ptr) {
        pr_err("kp_hook: stop_machine not found\n");
        return -ENOENT;
    }

    if (!kallsyms_lookup_size_offset_ptr)
        pr_warn("kp_hook: kallsyms_lookup_size_offset not found, skip length check\n");
    if (!stack_trace_save_tsk_ptr)
        pr_warn("kp_hook: stack_trace_save_tsk not found, skip stack safety check\n");

    g_runtime_ready = true;
    pr_info("kp_hook: runtime ready\n");
    return 0;
}

void kp_hook_runtime_exit(void)
{
    hijack_target_disable_all(true, NULL);
    g_runtime_ready = false;
}

int hijack_target_prepare(void *target, void *hook_dest,
    void *hook_template_code_space, void *hook_func, char *mod_name)
{
    struct sym_hook *sa = NULL;
    uint32_t ptr_hash;
    char *name = NULL;

    target += HOOK_TARGET_OFFSET;
    target = follow_trampoline(target);
    ptr_hash = jhash_pointer(target);

    if (!check_function_length_enough(target)) {
        pr_err("kp_hook: %lx shorter than hijack_size %d\n",
            (unsigned long)target, HIJACK_SIZE);
        return -1;
    }

    if (hook_template_code_space && !check_target_can_hijack(target)) {
        pr_err("kp_hook: %lx contains unhookable instructions\n",
            (unsigned long)target);
        return -1;
    }

    down_read(&hijack_targets_hashtable_lock);
    hash_for_each_possible(all_hijack_targets, sa, node, ptr_hash) {
        if (target == sa->target) {
            up_read(&hijack_targets_hashtable_lock);
            pr_err("kp_hook: %lx already prepared\n", (unsigned long)target);
            return -1;
        }
    }
    up_read(&hijack_targets_hashtable_lock);

    sa = kmalloc(sizeof(*sa), GFP_KERNEL);
    name = mod_name ? kstrdup(mod_name, GFP_KERNEL) : NULL;
    if (!sa || (mod_name && !name)) {
        kfree(sa);
        kfree(name);
        pr_err("kp_hook: no memory for %lx\n", (unsigned long)target);
        return -ENOMEM;
    }

    sa->target = target;
    memcpy(sa->target_code, target, HIJACK_SIZE);
    sa->hook_dest = hook_dest;
    sa->hook_template_code_space = hook_template_code_space;
    sa->hook_func = hook_func;
    sa->mod_name = name;
    sa->template_return_addr = target + HIJACK_SIZE - 1 * INSTRUCTION_SIZE;
    sa->enabled = false;

    down_write(&hijack_targets_hashtable_lock);
    hash_add(all_hijack_targets, &sa->node, ptr_hash);
    up_write(&hijack_targets_hashtable_lock);

    return 0;
}

static __nocfi int call_stop_machine(int (*fn)(void *), void *data)
{
    return stop_machine_ptr(fn, data, NULL);
}

int hijack_target_enable(void *target)
{
    struct sym_hook *sa;
    struct hlist_node *tmp;
    uint32_t ptr_hash;
    int ret = -1;
    unsigned char source_code[HIJACK_SIZE];
    struct do_hijack_struct dhs;

    target += HOOK_TARGET_OFFSET;
    target = follow_trampoline(target);
    ptr_hash = jhash_pointer(target);

    memset(source_code, 0, sizeof(source_code));
    dhs.dest = target;
    dhs.source = source_code;

    down_write(&hijack_targets_hashtable_lock);
    hash_for_each_possible_safe(all_hijack_targets, sa, tmp, node, ptr_hash) {
        if (sa->target == target) {
            if (!sa->enabled) {
                if (sa->hook_template_code_space &&
                    fill_hook_template_code_space(sa->hook_template_code_space,
                        sa->target_code, sa->template_return_addr)) {
                    goto out;
                }
                memcpy(source_code, sa->target_code, HIJACK_SIZE);
                fill_long_jmp(source_code, sa->hook_dest);
                dhs.hook_func = sa->hook_func;
                ret = call_stop_machine(do_hijack_target, &dhs);
                if (!ret)
                    sa->enabled = true;
            } else {
                pr_info("kp_hook: %lx already enabled\n", (unsigned long)target);
                ret = 0;
            }
            goto out;
        }
    }
    pr_err("kp_hook: %lx not prepared\n", (unsigned long)target);
out:
    up_write(&hijack_targets_hashtable_lock);
    return ret;
}

int hijack_target_disable(void *target, bool need_remove)
{
    struct sym_hook *sa;
    struct hlist_node *tmp;
    uint32_t ptr_hash;
    int ret = -1;
    struct do_hijack_struct dhs;

    target += HOOK_TARGET_OFFSET;
    target = follow_trampoline(target);
    ptr_hash = jhash_pointer(target);
    dhs.dest = target;

    down_write(&hijack_targets_hashtable_lock);
    hash_for_each_possible_safe(all_hijack_targets, sa, tmp, node, ptr_hash) {
        if (sa->target == target) {
            dhs.source = sa->target_code;
            dhs.hook_func = sa->hook_func;
            ret = call_stop_machine(do_hijack_target, &dhs);
            switch (ret) {
            case 0:
            case -1:
                sa->enabled = false;
                break;
            case -2:
            case -3:
                break;
            }
            if (need_remove && !ret) {
                hash_del(&sa->node);
                kfree(sa->mod_name);
                kfree(sa);
            }
            goto out;
        }
    }
    pr_err("kp_hook: %lx not prepared\n", (unsigned long)target);
out:
    up_write(&hijack_targets_hashtable_lock);
    return ret;
}

void hijack_target_disable_all(bool need_remove, char *mod_name)
{
    struct sym_hook *sa;
    struct hlist_node *tmp;
    int bkt, ret;
    bool retry;
    struct do_hijack_struct dhs;

    do {
        retry = false;
        down_write(&hijack_targets_hashtable_lock);
        hash_for_each_safe(all_hijack_targets, bkt, tmp, sa, node) {
            if (mod_name && sa->mod_name && strcmp(sa->mod_name, mod_name))
                continue;
            dhs.dest = sa->target;
            dhs.source = sa->target_code;
            dhs.hook_func = sa->hook_func;
            ret = call_stop_machine(do_hijack_target, &dhs);
            switch (ret) {
            case 0:
                sa->enabled = false;
                break;
            case -1:
                sa->enabled = false;
                retry = true;
                break;
            case -2:
            case -3:
                retry = true;
                break;
            }
            if (need_remove && !ret) {
                hash_del(&sa->node);
                kfree(sa->mod_name);
                kfree(sa);
            }
        }
        up_write(&hijack_targets_hashtable_lock);
    } while (retry && (msleep(1000), true));
}
