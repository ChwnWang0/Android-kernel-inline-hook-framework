#include "kp_hook.h"

#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>

#define SIMPLE_HOOK_ORIGIN_INST 4
#define ARM64_BTI_C 0xd503245f
#define ARM64_PACIASP 0xd503233f
#define ARM64_PACIBSP 0xd503237f

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
typedef int (*aarch64_insn_patch_text_t)(void *addrs[], u32 insns[], int cnt);
typedef int (*aarch64_insn_patch_text_nosync_t)(void *addr, u32 insn);
typedef void *(*module_alloc_t)(unsigned long size);
typedef void (*module_memfree_t)(void *region);
typedef int (*set_memory_x_t)(unsigned long addr, int numpages);

typedef int (*origin_func2_t)(u64, u64);

typedef struct {
    bool installed;
    void *func;
    void *origin;
    void *entry;
    void *udata;
    hook_chain2_callback before;
    hook_chain2_callback after;
    size_t alloc_size;
    int patched_inst_count;
    u32 origin_insts[SIMPLE_HOOK_ORIGIN_INST];
    u32 tramp_insts[SIMPLE_HOOK_ORIGIN_INST];
    u32 *relo;
} simple_hook_ctx_t;

struct kp_runtime_syms {
    kallsyms_lookup_name_t lookup_name;
    aarch64_insn_patch_text_t patch_text;
    aarch64_insn_patch_text_nosync_t patch_text_nosync;
    module_alloc_t module_alloc;
    module_memfree_t module_memfree;
    set_memory_x_t set_memory_x;
};

static struct kp_runtime_syms g_syms;
static bool g_runtime_ready;
static simple_hook_ctx_t g_hook;

static unsigned long kp_lookup_name_raw(const char *name);
static void free_exec(void *mem);

static u64 bits32(u32 n, u32 high, u32 low)
{
    return ((u32)(n << (31u - high))) >> (31u - high + low);
}

static u64 sign64_extend(u64 n, u32 len)
{
    return (((u64)(n << (63u - (len - 1))) >> 63u) ? (n | (~0ULL << len)) : n);
}

static int __nocfi kp_call_set_memory_x(set_memory_x_t func, unsigned long addr, int numpages)
{
    return func(addr, numpages);
}

static unsigned long kp_lookup_name(const char *name)
{
    char cfi_name[128];
    unsigned long addr;

    snprintf(cfi_name, sizeof(cfi_name), "%s.cfi_jt", name);
    addr = kp_lookup_name_raw(cfi_name);
    if (addr)
        return addr;

    return kp_lookup_name_raw(name);
}

static unsigned long kp_lookup_name_raw(const char *name)
{
    struct kprobe kp = {
        .symbol_name = name,
    };
    unsigned long addr = 0;

    if (register_kprobe(&kp) == 0) {
        addr = (unsigned long)kp.addr;
        unregister_kprobe(&kp);
        return addr;
    }

    if (!g_syms.lookup_name && strcmp(name, "kallsyms_lookup_name") && strcmp(name, "kallsyms_lookup_name.cfi_jt")) {
        addr = kp_lookup_name_raw("kallsyms_lookup_name.cfi_jt");
        if (!addr)
            addr = kp_lookup_name_raw("kallsyms_lookup_name");
        if (addr)
            g_syms.lookup_name = (kallsyms_lookup_name_t)addr;
    }

    if (g_syms.lookup_name)
        return g_syms.lookup_name(name);

    return 0;
}

static void *alloc_exec(size_t size, size_t *alloc_size)
{
    size_t aligned = PAGE_ALIGN(size);
    void *mem;
    int ret;

    if (!g_syms.module_alloc)
        return NULL;

    mem = g_syms.module_alloc(aligned);
    if (!mem)
        return NULL;

    memset(mem, 0, aligned);
    if (g_syms.set_memory_x) {
        ret = kp_call_set_memory_x(g_syms.set_memory_x, (unsigned long)mem, aligned >> PAGE_SHIFT);
        if (ret) {
            pr_err("kp_hook: set_memory_x failed for %p, ret=%d\n", mem, ret);
            free_exec(mem);
            return NULL;
        }
    }

    if (alloc_size)
        *alloc_size = aligned;
    return mem;
}

static void free_exec(void *mem)
{
    if (!mem)
        return;
    if (g_syms.module_memfree)
        g_syms.module_memfree(mem);
    else
        vfree(mem);
}

static int patch_text_single_nosync(void *addr, u32 value)
{
    if (g_syms.patch_text_nosync)
        return g_syms.patch_text_nosync(addr, value);

    *(u32 *)addr = value;
    flush_icache_range((unsigned long)addr, (unsigned long)addr + sizeof(u32));
    return 0;
}

static int patch_text_array(void *addrs[], u32 values[], int cnt)
{
    int i;
    int ret = 0;

    if (cnt <= 0)
        return -EINVAL;

    if (g_syms.patch_text)
        return g_syms.patch_text(addrs, values, cnt);

    for (i = 0; i < cnt; i++) {
        ret = patch_text_single_nosync(addrs[i], values[i]);
        if (ret)
            return ret;
    }

    return 0;
}

static u64 branch_func_addr_once(u64 addr)
{
    u32 inst = *(u32 *)addr;

    if ((inst & 0xFC000000u) == 0x14000000u) {
        u64 imm26 = bits32(inst, 25, 0);
        u64 imm64 = sign64_extend(imm26 << 2u, 28u);

        return addr + imm64;
    }

    if (inst == 0xd503245f || inst == 0xd503249f || inst == 0xd50324df)
        return addr + 4;

    return addr;
}

static u64 branch_func_addr(u64 addr)
{
    u64 next;

    for (;;) {
        next = branch_func_addr_once(addr);
        if (next == addr)
            return addr;
        addr = next;
    }
}

static int32_t branch_absolute(u32 *buf, u64 addr)
{
    buf[0] = 0x58000051;
    buf[1] = 0xd61f0220;
    buf[2] = (u32)(addr & 0xffffffffu);
    buf[3] = (u32)(addr >> 32u);
    return 4;
}

static int32_t ret_absolute(u32 *buf, u64 addr)
{
    buf[0] = 0x58000051;
    buf[1] = 0xD65F0220;
    buf[2] = (u32)(addr & 0xffffffffu);
    buf[3] = (u32)(addr >> 32u);
    return 4;
}

static int build_relo_for_simple_target(simple_hook_ctx_t *ctx)
{
    int i;
    u32 *relo = ctx->relo;

    relo[0] = ARM64_BTI_C;

    for (i = 0; i < SIMPLE_HOOK_ORIGIN_INST; i++) {
        u32 inst = ctx->origin_insts[i];

        if ((inst & 0x9f000000u) == 0x10000000u ||
            (inst & 0x9f000000u) == 0x90000000u ||
            (inst & 0xfc000000u) == 0x14000000u ||
            (inst & 0xfc000000u) == 0x94000000u ||
            (inst & 0xff000010u) == 0x54000000u ||
            (inst & 0x7f000000u) == 0x34000000u ||
            (inst & 0x7f000000u) == 0x35000000u ||
            (inst & 0x7f000000u) == 0x36000000u ||
            (inst & 0x7f000000u) == 0x37000000u ||
            (inst & 0xff000000u) == 0x18000000u ||
            (inst & 0xff000000u) == 0x58000000u ||
            (inst & 0xff000000u) == 0x98000000u ||
            (inst & 0xff000000u) == 0xD8000000u ||
            (inst & 0xff000000u) == 0x1C000000u ||
            (inst & 0xff000000u) == 0x5C000000u ||
            (inst & 0xff000000u) == 0x9C000000u) {
            pr_err("kp_hook: unsupported prologue inst[%d]=0x%08x\n", i, inst);
            return -HOOK_BAD_RELO;
        }

        relo[i + 1] = inst;
    }

    ret_absolute(&relo[SIMPLE_HOOK_ORIGIN_INST + 1], (u64)ctx->origin + SIMPLE_HOOK_ORIGIN_INST * sizeof(u32));
    flush_icache_range((unsigned long)relo,
                       (unsigned long)relo + (SIMPLE_HOOK_ORIGIN_INST + 1 + 4) * sizeof(u32));
    return HOOK_NO_ERR;
}

static int call_origin_add(u64 arg0, u64 arg1)
{
    origin_func2_t origin = (origin_func2_t)g_hook.relo;

    return origin(arg0, arg1);
}

static __nocfi int hook_dispatch_add(int a, int b)
{
    hook_fargs2_t fargs;

    if (!g_hook.installed)
        return call_origin_add(a, b);

    memset(&fargs, 0, sizeof(fargs));
    fargs.chain = &g_hook;
    fargs.arg0 = (u64)a;
    fargs.arg1 = (u64)b;

    if (g_hook.before)
        g_hook.before(&fargs, g_hook.udata);

    if (!fargs.skip_origin)
        fargs.ret = call_origin_add(fargs.arg0, fargs.arg1);

    if (g_hook.after)
        g_hook.after(&fargs, g_hook.udata);

    return (int)fargs.ret;
}

static int install_simple_hook(simple_hook_ctx_t *ctx)
{
    void *addrs[SIMPLE_HOOK_ORIGIN_INST];
    int i;
    int ret;

    if (ctx->origin_insts[0] == ARM64_PACIASP || ctx->origin_insts[0] == ARM64_PACIBSP) {
        pr_err("kp_hook: PAC prologue is not supported by simple hook\n");
        return -EINVAL;
    }

    branch_absolute(ctx->tramp_insts, (u64)hook_dispatch_add);
    ctx->patched_inst_count = 4;

    for (i = 0; i < ctx->patched_inst_count; i++)
        addrs[i] = (u32 *)ctx->origin + i;

    ret = patch_text_array(addrs, ctx->tramp_insts, ctx->patched_inst_count);
    if (!ret)
        flush_icache_range((unsigned long)ctx->origin,
                           (unsigned long)ctx->origin + ctx->patched_inst_count * sizeof(u32));
    return ret;
}

static void uninstall_simple_hook(simple_hook_ctx_t *ctx)
{
    void *addrs[SIMPLE_HOOK_ORIGIN_INST];
    int i;

    for (i = 0; i < ctx->patched_inst_count; i++)
        addrs[i] = (u32 *)ctx->origin + i;

    patch_text_array(addrs, ctx->origin_insts, ctx->patched_inst_count);
    flush_icache_range((unsigned long)ctx->origin,
                       (unsigned long)ctx->origin + ctx->patched_inst_count * sizeof(u32));
}

int kp_hook_runtime_init(void)
{
    if (g_runtime_ready)
        return 0;

    memset(&g_syms, 0, sizeof(g_syms));
    memset(&g_hook, 0, sizeof(g_hook));

    g_syms.module_alloc = (module_alloc_t)kp_lookup_name("module_alloc");
    g_syms.module_memfree = (module_memfree_t)kp_lookup_name("module_memfree");
    g_syms.set_memory_x = (set_memory_x_t)kp_lookup_name("set_memory_x");
    g_syms.patch_text = (aarch64_insn_patch_text_t)kp_lookup_name("aarch64_insn_patch_text");
    g_syms.patch_text_nosync = (aarch64_insn_patch_text_nosync_t)kp_lookup_name("aarch64_insn_patch_text_nosync");

    if (!g_syms.module_alloc) {
        pr_err("kp_hook: failed to resolve module_alloc\n");
        return -ENOENT;
    }

    if (!g_syms.patch_text && !g_syms.patch_text_nosync) {
        pr_err("kp_hook: text patch helpers not found\n");
        return -ENOENT;
    }

    g_runtime_ready = true;
    pr_info("kp_hook: simple runtime ready\n");
    return 0;
}

void kp_hook_runtime_exit(void)
{
    if (g_hook.installed)
        uninstall_simple_hook(&g_hook);

    if (g_hook.entry)
        free_exec(g_hook.entry);

    memset(&g_hook, 0, sizeof(g_hook));
    g_runtime_ready = false;
}

hook_err_t hook_wrap(void *func, s32 argno, void *before, void *after, void *udata)
{
    simple_hook_ctx_t *ctx = &g_hook;
    int i;
    int ret;

    if (!g_runtime_ready)
        return -HOOK_INIT_FAILED;
    if (!func)
        return -HOOK_BAD_ADDRESS;
    if (argno != 2)
        return -HOOK_BAD_RELO;
    if (ctx->installed)
        return -HOOK_DUPLICATED;

    memset(ctx, 0, sizeof(*ctx));
    ctx->func = func;
    ctx->origin = (void *)branch_func_addr((u64)func);
    ctx->before = (hook_chain2_callback)before;
    ctx->after = (hook_chain2_callback)after;
    ctx->udata = udata;

    for (i = 0; i < SIMPLE_HOOK_ORIGIN_INST; i++)
        ctx->origin_insts[i] = *((u32 *)ctx->origin + i);

    pr_info("kp_hook: target %p prologue %08x %08x %08x %08x\n",
            ctx->origin, ctx->origin_insts[0], ctx->origin_insts[1],
            ctx->origin_insts[2], ctx->origin_insts[3]);

    ctx->entry = alloc_exec(PAGE_SIZE, &ctx->alloc_size);
    if (!ctx->entry)
        return -HOOK_NO_MEM;

    ctx->relo = (u32 *)ctx->entry;
    ret = build_relo_for_simple_target(ctx);
    if (ret) {
        free_exec(ctx->entry);
        memset(ctx, 0, sizeof(*ctx));
        return ret;
    }

    ret = install_simple_hook(ctx);
    if (ret) {
        free_exec(ctx->entry);
        memset(ctx, 0, sizeof(*ctx));
        return -HOOK_BAD_RELO;
    }

    ctx->installed = true;
    pr_info("kp_hook: simple hook installed for %p\n", func);
    return HOOK_NO_ERR;
}

void hook_unwrap_remove(void *func, void *before, void *after, int remove)
{
    simple_hook_ctx_t *ctx = &g_hook;

    if (!remove || !ctx->installed)
        return;
    if (func && ctx->func != func)
        return;
    if (before && ctx->before != (hook_chain2_callback)before)
        return;
    if (after && ctx->after != (hook_chain2_callback)after)
        return;

    uninstall_simple_hook(ctx);
    if (ctx->entry)
        free_exec(ctx->entry);
    memset(ctx, 0, sizeof(*ctx));
    pr_info("kp_hook: simple hook removed\n");
}
