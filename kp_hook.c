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

/* Instruction manipulation macros */
#define bits32(n, high, low) ((uint32_t)((n) << (31u - (high))) >> (31u - (high) + (low)))
#define bit(n, st) (((n) >> (st)) & 1)
#define sign64_extend(n, len) \
    (((uint64_t)((n) << (63u - (len - 1))) >> 63u) ? ((n) | (0xFFFFFFFFFFFFFFFFULL << (len))) : (n))

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

/* Relocate B/BL/B.cond instruction */
static int relo_b(struct sym_hook *hook, uint64_t inst_addr, uint32_t inst)
{
    uint32_t *buf;
    uint64_t imm64;
    uint64_t addr;
    int idx;
    bool is_bl;
    bool is_bc;
    uint64_t imm19;
    uint64_t imm26;

    buf = hook->relo_insts + hook->relo_insts_num;
    idx = 0;
    is_bl = (inst & MASK_BL) == INST_BL;
    is_bc = (inst & MASK_BC) == INST_BC;

    if (is_bc) {
        imm19 = bits32(inst, 23, 5);
        imm64 = sign64_extend(imm19 << 2u, 21u);
    } else {
        imm26 = bits32(inst, 25, 0);
        imm64 = sign64_extend(imm26 << 2u, 28u);
    }
    addr = inst_addr + imm64;

    if (is_bc) {
        buf[idx++] = (inst & 0xFF00001F) | 0x40u; /* B.<cond> #8 */
        buf[idx++] = 0x14000006; /* B #24 */
    }
    buf[idx++] = 0x58000051; /* LDR X17, #8 */
    buf[idx++] = 0x14000003; /* B #12 */
    buf[idx++] = addr & 0xFFFFFFFF;
    buf[idx++] = addr >> 32u;
    if (is_bl) {
        buf[idx++] = 0x1000001E; /* ADR X30, . */
        buf[idx++] = 0x910033DE; /* ADD X30, X30, #12 */
        buf[idx++] = 0xD65F0220; /* RET X17 */
    } else {
        buf[idx++] = 0xD65F0220; /* RET X17 */
    }
    buf[idx++] = ARM64_NOP;

    hook->relo_insts_num += idx;
    return 0;
}

/* Relocate ADR/ADRP instruction */
static int relo_adr(struct sym_hook *hook, uint64_t inst_addr, uint32_t inst)
{
    uint32_t *buf;
    uint32_t xd;
    uint64_t immlo;
    uint64_t immhi;
    uint64_t addr;
    bool is_adrp;

    buf = hook->relo_insts + hook->relo_insts_num;
    xd = bits32(inst, 4, 0);
    immlo = bits32(inst, 30, 29);
    immhi = bits32(inst, 23, 5);
    is_adrp = (inst & MASK_ADRP) == INST_ADRP;

    if (is_adrp) {
        addr = (inst_addr + sign64_extend((immhi << 14u) | (immlo << 12u), 33u)) & 0xFFFFFFFFFFFFF000ULL;
    } else {
        addr = inst_addr + sign64_extend((immhi << 2u) | immlo, 21u);
    }

    buf[0] = 0x58000040u | xd; /* LDR Xd, #8 */
    buf[1] = 0x14000003; /* B #12 */
    buf[2] = addr & 0xFFFFFFFF;
    buf[3] = addr >> 32u;

    hook->relo_insts_num += 4;
    return 0;
}

/* Relocate LDR literal instruction */
static int relo_ldr(struct sym_hook *hook, uint64_t inst_addr, uint32_t inst)
{
    uint32_t *buf;
    uint32_t rt;
    uint64_t imm19;
    uint64_t offset;
    uint64_t addr;
    bool is_ldr32;
    bool is_ldr64;
    bool is_ldrsw;

    buf = hook->relo_insts + hook->relo_insts_num;
    rt = bits32(inst, 4, 0);
    imm19 = bits32(inst, 23, 5);
    offset = sign64_extend((imm19 << 2u), 21u);
    addr = inst_addr + offset;
    is_ldr32 = (inst & MASK_LDR_32) == INST_LDR_32;
    is_ldr64 = (inst & MASK_LDR_64) == INST_LDR_64;
    is_ldrsw = (inst & MASK_LDRSW_LIT) == INST_LDRSW_LIT;

    if (is_ldr32 || is_ldr64 || is_ldrsw) {
        buf[0] = 0x58000060u | rt; /* LDR Xt, #12 */
        if (is_ldr32) {
            buf[1] = 0xB9400000 | rt | (rt << 5u); /* LDR Wt, [Xt] */
        } else if (is_ldr64) {
            buf[1] = 0xF9400000 | rt | (rt << 5u); /* LDR Xt, [Xt] */
        } else {
            buf[1] = 0xB9800000 | rt | (rt << 5u); /* LDRSW Xt, [Xt] */
        }
        buf[2] = 0x14000004; /* B #16 */
        buf[3] = ARM64_NOP;
        buf[4] = addr & 0xFFFFFFFF;
        buf[5] = addr >> 32u;
        hook->relo_insts_num += 6;
    }

    return 0;
}

/* Relocate CBZ/CBNZ/TBZ/TBNZ instruction */
static int relo_cbz(struct sym_hook *hook, uint64_t inst_addr, uint32_t inst)
{
    uint32_t *buf;
    uint64_t imm19;
    uint64_t imm64;
    uint64_t addr;

    buf = hook->relo_insts + hook->relo_insts_num;
    imm19 = bits32(inst, 23, 5);
    imm64 = sign64_extend(imm19 << 2u, 21u);
    addr = inst_addr + imm64;

    buf[0] = (inst & 0xFF00001F) | 0x40u; /* CBZ/CBNZ/TBZ/TBNZ #8 */
    buf[1] = 0x14000006; /* B #24 */
    buf[2] = 0x58000051; /* LDR X17, #8 */
    buf[3] = 0x14000003; /* B #12 */
    buf[4] = addr & 0xFFFFFFFF;
    buf[5] = addr >> 32u;
    buf[6] = 0xD65F0220; /* RET X17 */
    buf[7] = ARM64_NOP;

    hook->relo_insts_num += 8;
    return 0;
}

/* Relocate single instruction */
static int relo_single_inst(struct sym_hook *hook, int inst_idx)
{
    uint32_t inst = hook->origin_insts[inst_idx];
    uint64_t inst_addr = (uint64_t)hook->target + inst_idx * INSTRUCTION_SIZE;

    /* BTI instructions: copy as-is */
    if (inst == ARM64_BTI_C || inst == ARM64_BTI_J || inst == ARM64_BTI_JC) {
        hook->relo_insts[hook->relo_insts_num++] = inst;
        return 0;
    }

    /* Check instruction type and relocate */
    if ((inst & MASK_B) == INST_B || (inst & MASK_BL) == INST_BL || (inst & MASK_BC) == INST_BC) {
        return relo_b(hook, inst_addr, inst);
    }
    if ((inst & MASK_ADR) == INST_ADR || (inst & MASK_ADRP) == INST_ADRP) {
        return relo_adr(hook, inst_addr, inst);
    }
    if ((inst & MASK_LDR_32) == INST_LDR_32 || (inst & MASK_LDR_64) == INST_LDR_64 ||
        (inst & MASK_LDRSW_LIT) == INST_LDRSW_LIT) {
        return relo_ldr(hook, inst_addr, inst);
    }
    if ((inst & MASK_CBZ) == INST_CBZ || (inst & MASK_CBNZ) == INST_CBNZ ||
        (inst & MASK_TBZ) == INST_TBZ || (inst & MASK_TBNZ) == INST_TBNZ) {
        return relo_cbz(hook, inst_addr, inst);
    }

    /* Non-PC-relative instruction: copy as-is */
    hook->relo_insts[hook->relo_insts_num++] = inst;
    return 0;
}

/* Build relocated instruction sequence */
static int build_relo_insts(struct sym_hook *hook)
{
    int i;
    uint32_t *buf;
    uint64_t ret_addr;

    hook->relo_insts_num = 0;
    hook->tramp_insts_num = HIJACK_INST_NUM;

    /* Copy original instructions */
    for (i = 0; i < HIJACK_INST_NUM; i++) {
        hook->origin_insts[i] = *(uint32_t *)(hook->target + i * INSTRUCTION_SIZE);
    }

    /* Relocate each instruction */
    for (i = 0; i < HIJACK_INST_NUM; i++) {
        if (relo_single_inst(hook, i) < 0) {
            pr_err("kp_hook: failed to relocate instruction at offset %d\n", i * INSTRUCTION_SIZE);
            return -1;
        }
    }

    /* Add long jump back to original function */
    buf = hook->relo_insts + hook->relo_insts_num;
    ret_addr = (uint64_t)hook->target + HIJACK_SIZE;

    buf[0] = 0x58000051; /* LDR X17, #8 */
    buf[1] = 0x14000003; /* B #12 */
    buf[2] = ret_addr & 0xFFFFFFFF;
    buf[3] = ret_addr >> 32u;
    buf[4] = 0xD65F0220; /* RET X17 */
    buf[5] = ARM64_NOP;

    hook->relo_insts_num += 6;

    pr_info("kp_hook: built %d relocated instructions\n", hook->relo_insts_num);
    return 0;
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

static int fill_hook_template_code_space(struct sym_hook *sa)
{
    /* Write relocated instructions to code_space */
    int total_size = sa->relo_insts_num * INSTRUCTION_SIZE;
    return hook_write_range(sa->hook_template_code_space, sa->relo_insts, total_size);
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

    /* Build relocated instructions if code_space is provided */
    if (hook_template_code_space) {
        if (build_relo_insts(sa) < 0) {
            kfree(sa->mod_name);
            kfree(sa);
            pr_err("kp_hook: failed to build relocated instructions for %lx\n",
                (unsigned long)target);
            return -1;
        }
    }

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
                    fill_hook_template_code_space(sa)) {
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
