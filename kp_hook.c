#include "kp_hook.h"

#include <linux/cpumask.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/stop_machine.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>

#define bits32(n, high, low) ((u32)((n) << (31u - (high))) >> (31u - (high) + (low)))
#define sign64_extend(n, len) \
    (((u64)((n) << (63u - ((len) - 1))) >> 63u) ? ((n) | (~0ULL << (len))) : (n))

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
typedef int (*aarch64_insn_patch_text_t)(void *addrs[], u32 insns[], int cnt);
typedef int (*aarch64_insn_patch_text_nosync_t)(void *addr, u32 insn);
typedef void *(*module_alloc_t)(unsigned long size);
typedef void (*module_memfree_t)(void *region);
typedef int (*set_memory_x_t)(unsigned long addr, int numpages);

struct kp_runtime_syms {
    kallsyms_lookup_name_t lookup_name;
    aarch64_insn_patch_text_t patch_text;
    aarch64_insn_patch_text_nosync_t patch_text_nosync;
    module_alloc_t module_alloc;
    module_memfree_t module_memfree;
    set_memory_x_t set_memory_x;
};

struct patch_request {
    void **addrs;
    u32 *values;
    int cnt;
    int ret;
};

struct hook_entry {
    struct list_head node;
    enum hook_type type;
    unsigned long key;
    void *ctx;
    size_t alloc_size;
};

static struct kp_runtime_syms g_syms;
static LIST_HEAD(g_hook_entries);
static DEFINE_MUTEX(g_hook_lock);
static bool g_runtime_ready;

static unsigned long kp_lookup_name_raw(const char *name);
static void free_exec(void *mem);

static int __nocfi kp_call_set_memory_x(set_memory_x_t func, unsigned long addr, int numpages)
{
    return func(addr, numpages);
}

static inline void dsb_ish(void)
{
    asm volatile("dsb ish" : : : "memory");
}

static inline void isb_sy(void)
{
    asm volatile("isb" : : : "memory");
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
    } else {
        pr_warn("kp_hook: set_memory_x not found, allocated memory may be NX\n");
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

static int patch_request_cb(void *arg)
{
    struct patch_request *req = arg;
    int i;

    req->ret = 0;
    for (i = 0; i < req->cnt && !req->ret; i++)
        req->ret = patch_text_single_nosync(req->addrs[i], req->values[i]);
    return req->ret;
}

static int patch_text_array(void *addrs[], u32 values[], int cnt)
{
    struct patch_request req = {
        .addrs = addrs,
        .values = values,
        .cnt = cnt,
        .ret = 0,
    };

    if (cnt <= 0)
        return -EINVAL;

    if (g_syms.patch_text)
        return g_syms.patch_text(addrs, values, cnt);

    return stop_machine(patch_request_cb, &req, cpu_online_mask);
}

static inline int is_bad_address(const void *addr)
{
    return !addr || (((u64)addr & (1ULL << 63)) == 0);
}

static struct hook_entry *find_entry_locked(unsigned long key)
{
    struct hook_entry *entry;

    list_for_each_entry(entry, &g_hook_entries, node) {
        if (entry->key == key)
            return entry;
    }

    return NULL;
}

static int add_entry_locked(enum hook_type type, unsigned long key, void *ctx, size_t alloc_size)
{
    struct hook_entry *entry;

    entry = kzalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry)
        return -ENOMEM;

    entry->type = type;
    entry->key = key;
    entry->ctx = ctx;
    entry->alloc_size = alloc_size;
    list_add(&entry->node, &g_hook_entries);
    return 0;
}

static void del_entry_locked(struct hook_entry *entry)
{
    list_del(&entry->node);
    kfree(entry);
}

#ifdef HOOK_INTO_BRANCH_FUNC
static u64 branch_func_addr_once(u64 addr)
{
    u32 inst = *(u32 *)addr;

    if ((inst & 0xFC000000u) == 0x14000000u) {
        u64 imm26 = bits32(inst, 25, 0);
        u64 imm64 = sign64_extend(imm26 << 2u, 28u);
        return addr + imm64;
    }

    if (inst == ARM64_BTI_C || inst == ARM64_BTI_J || inst == ARM64_BTI_JC)
        return addr + 4;

    return addr;
}

static u64 branch_func_addr(u64 addr)
{
    for (;;) {
        u64 next = branch_func_addr_once(addr);

        if (next == addr)
            return addr;
        addr = next;
    }
}
#else
static u64 branch_func_addr(u64 addr)
{
    return addr;
}
#endif

typedef u32 inst_type_t;
typedef u32 inst_mask_t;

#define INST_B 0x14000000
#define INST_BC 0x54000000
#define INST_BL 0x94000000
#define INST_ADR 0x10000000
#define INST_ADRP 0x90000000
#define INST_LDR_32 0x18000000
#define INST_LDR_64 0x58000000
#define INST_LDRSW_LIT 0x98000000
#define INST_PRFM_LIT 0xD8000000
#define INST_LDR_SIMD_32 0x1C000000
#define INST_LDR_SIMD_64 0x5C000000
#define INST_LDR_SIMD_128 0x9C000000
#define INST_CBZ 0x34000000
#define INST_CBNZ 0x35000000
#define INST_TBZ 0x36000000
#define INST_TBNZ 0x37000000
#define INST_IGNORE 0x0

#define MASK_B 0xFC000000
#define MASK_BC 0xFF000010
#define MASK_BL 0xFC000000
#define MASK_ADR 0x9F000000
#define MASK_ADRP 0x9F000000
#define MASK_LDR_32 0xFF000000
#define MASK_LDR_64 0xFF000000
#define MASK_LDRSW_LIT 0xFF000000
#define MASK_PRFM_LIT 0xFF000000
#define MASK_LDR_SIMD_32 0xFF000000
#define MASK_LDR_SIMD_64 0xFF000000
#define MASK_LDR_SIMD_128 0xFF000000
#define MASK_CBZ 0x7F000000u
#define MASK_CBNZ 0x7F000000u
#define MASK_TBZ 0x7F000000u
#define MASK_TBNZ 0x7F000000u
#define MASK_IGNORE 0x0

static inst_mask_t masks[] = {
    MASK_B,      MASK_BC,        MASK_BL,       MASK_ADR,         MASK_ADRP,        MASK_LDR_32,
    MASK_LDR_64, MASK_LDRSW_LIT, MASK_PRFM_LIT, MASK_LDR_SIMD_32, MASK_LDR_SIMD_64, MASK_LDR_SIMD_128,
    MASK_CBZ,    MASK_CBNZ,      MASK_TBZ,      MASK_TBNZ,        MASK_IGNORE,
};

static inst_type_t types[] = {
    INST_B,      INST_BC,        INST_BL,       INST_ADR,         INST_ADRP,        INST_LDR_32,
    INST_LDR_64, INST_LDRSW_LIT, INST_PRFM_LIT, INST_LDR_SIMD_32, INST_LDR_SIMD_64, INST_LDR_SIMD_128,
    INST_CBZ,    INST_CBNZ,      INST_TBZ,      INST_TBNZ,        INST_IGNORE,
};

static int relo_len[] = { 6, 8, 8, 4, 4, 6, 6, 6, 8, 8, 8, 8, 6, 6, 6, 6, 2 };

static int is_in_tramp(hook_t *hook, u64 addr)
{
    u64 tramp_start = hook->origin_addr;
    u64 tramp_end = tramp_start + hook->tramp_insts_num * 4;

    return addr >= tramp_start && addr < tramp_end;
}

static u64 relo_in_tramp(hook_t *hook, u64 addr)
{
    u64 tramp_start = hook->origin_addr;
    u64 tramp_end = tramp_start + hook->tramp_insts_num * 4;
    u32 addr_inst_index;
    u64 fix_addr;
    int i;
    int j;

    if (!(addr >= tramp_start && addr < tramp_end))
        return addr;

    addr_inst_index = (addr - tramp_start) / 4;
    fix_addr = hook->relo_addr;
    for (i = 0; i < addr_inst_index; i++) {
        inst_type_t inst = hook->origin_insts[i];

        for (j = 0; j < ARRAY_SIZE(relo_len); j++) {
            if ((inst & masks[j]) == types[j]) {
                fix_addr += relo_len[j] * 4;
                break;
            }
        }
    }
    return fix_addr;
}

int32_t branch_relative(u32 *buf, u64 src_addr, u64 dst_addr)
{
#define B_REL_RANGE ((1 << 25) << 2)
    if (((dst_addr >= src_addr) && (dst_addr - src_addr <= B_REL_RANGE)) ||
        ((src_addr >= dst_addr) && (src_addr - dst_addr <= B_REL_RANGE))) {
        buf[0] = 0x14000000u | (((dst_addr - src_addr) & 0x0FFFFFFFu) >> 2u);
        buf[1] = ARM64_NOP;
        return 2;
    }

    return 0;
}

int32_t branch_absolute(u32 *buf, u64 addr)
{
    buf[0] = 0x58000051;
    buf[1] = 0xd61f0220;
    buf[2] = addr & 0xFFFFFFFF;
    buf[3] = addr >> 32u;
    return 4;
}

int32_t ret_absolute(u32 *buf, u64 addr)
{
    buf[0] = 0x58000051;
    buf[1] = 0xD65F0220;
    buf[2] = addr & 0xFFFFFFFF;
    buf[3] = addr >> 32u;
    return 4;
}

int32_t branch_from_to(u32 *buf, u64 src_addr, u64 dst_addr)
{
    return ret_absolute(buf, dst_addr);
}

static hook_err_t relo_b(hook_t *hook, u64 inst_addr, u32 inst, inst_type_t type)
{
    u32 *buf = hook->relo_insts + hook->relo_insts_num;
    u64 imm64;
    u64 addr;
    u32 idx = 0;

    if (type == INST_BC) {
        u64 imm19 = bits32(inst, 23, 5);

        imm64 = sign64_extend(imm19 << 2u, 21u);
    } else {
        u64 imm26 = bits32(inst, 25, 0);

        imm64 = sign64_extend(imm26 << 2u, 28u);
    }

    addr = relo_in_tramp(hook, inst_addr + imm64);

    if (type == INST_BC) {
        buf[idx++] = (inst & 0xFF00001F) | 0x40u;
        buf[idx++] = 0x14000006;
    }
    buf[idx++] = 0x58000051;
    buf[idx++] = 0x14000003;
    buf[idx++] = addr & 0xFFFFFFFF;
    buf[idx++] = addr >> 32u;
    if (type == INST_BL) {
        buf[idx++] = 0x1000001E;
        buf[idx++] = 0x910033DE;
        buf[idx++] = 0xD65F0220;
    } else {
        buf[idx++] = 0xD65F0220;
    }
    buf[idx++] = ARM64_NOP;
    return HOOK_NO_ERR;
}

static hook_err_t relo_adr(hook_t *hook, u64 inst_addr, u32 inst, inst_type_t type)
{
    u32 *buf = hook->relo_insts + hook->relo_insts_num;
    u32 xd = bits32(inst, 4, 0);
    u64 immlo = bits32(inst, 30, 29);
    u64 immhi = bits32(inst, 23, 5);
    u64 addr;

    if (type == INST_ADR) {
        addr = inst_addr + sign64_extend((immhi << 2u) | immlo, 21u);
    } else {
        addr = (inst_addr + sign64_extend((immhi << 14u) | (immlo << 12u), 33u)) & 0xFFFFFFFFFFFFF000ULL;
        if (is_in_tramp(hook, addr))
            return -HOOK_BAD_RELO;
    }

    buf[0] = 0x58000040u | xd;
    buf[1] = 0x14000003;
    buf[2] = addr & 0xFFFFFFFF;
    buf[3] = addr >> 32u;
    return HOOK_NO_ERR;
}

static hook_err_t relo_ldr(hook_t *hook, u64 inst_addr, u32 inst, inst_type_t type)
{
    u32 *buf = hook->relo_insts + hook->relo_insts_num;
    u32 rt = bits32(inst, 4, 0);
    u64 imm19 = bits32(inst, 23, 5);
    u64 offset = sign64_extend(imm19 << 2u, 21u);
    u64 addr = relo_in_tramp(hook, inst_addr + offset);

    if (is_in_tramp(hook, inst_addr + offset) && type != INST_PRFM_LIT)
        return -HOOK_BAD_RELO;

    if (type == INST_LDR_32 || type == INST_LDR_64 || type == INST_LDRSW_LIT) {
        buf[0] = 0x58000060u | rt;
        if (type == INST_LDR_32)
            buf[1] = 0xB9400000 | rt | (rt << 5u);
        else if (type == INST_LDR_64)
            buf[1] = 0xF9400000 | rt | (rt << 5u);
        else
            buf[1] = 0xB9800000 | rt | (rt << 5u);
        buf[2] = 0x14000004;
        buf[3] = ARM64_NOP;
        buf[4] = addr & 0xFFFFFFFF;
        buf[5] = addr >> 32u;
    } else {
        buf[0] = 0xA93F47F0;
        buf[1] = 0x58000091;
        if (type == INST_PRFM_LIT)
            buf[2] = 0xF9800220 | rt;
        else if (type == INST_LDR_SIMD_32)
            buf[2] = 0xBD400220 | rt;
        else if (type == INST_LDR_SIMD_64)
            buf[2] = 0xFD400220 | rt;
        else
            buf[2] = 0x3DC00220u | rt;
        buf[3] = 0xF85F83F1;
        buf[4] = 0x14000004;
        buf[5] = ARM64_NOP;
        buf[6] = addr & 0xFFFFFFFF;
        buf[7] = addr >> 32u;
    }

    return HOOK_NO_ERR;
}

static hook_err_t relo_cb(hook_t *hook, u64 inst_addr, u32 inst)
{
    u32 *buf = hook->relo_insts + hook->relo_insts_num;
    u64 imm19 = bits32(inst, 23, 5);
    u64 offset = sign64_extend(imm19 << 2u, 21u);
    u64 addr = relo_in_tramp(hook, inst_addr + offset);

    buf[0] = (inst & 0xFF00001F) | 0x40u;
    buf[1] = 0x14000005;
    buf[2] = 0x58000051;
    buf[3] = 0xD65F0220;
    buf[4] = addr & 0xFFFFFFFF;
    buf[5] = addr >> 32u;
    return HOOK_NO_ERR;
}

static hook_err_t relo_tb(hook_t *hook, u64 inst_addr, u32 inst)
{
    u32 *buf = hook->relo_insts + hook->relo_insts_num;
    u64 imm14 = bits32(inst, 18, 5);
    u64 offset = sign64_extend(imm14 << 2u, 16u);
    u64 addr = relo_in_tramp(hook, inst_addr + offset);

    buf[0] = (inst & 0xFFF8001F) | 0x40u;
    buf[1] = 0x14000005;
    buf[2] = 0x58000051;
    buf[3] = 0xD61F0220;
    buf[4] = addr & 0xFFFFFFFF;
    buf[5] = addr >> 32u;
    return HOOK_NO_ERR;
}

static hook_err_t relo_ignore(hook_t *hook, u32 inst)
{
    u32 *buf = hook->relo_insts + hook->relo_insts_num;

    buf[0] = inst;
    buf[1] = ARM64_NOP;
    return HOOK_NO_ERR;
}

static hook_err_t relocate_inst(hook_t *hook, u64 inst_addr, u32 inst)
{
    hook_err_t rc = HOOK_NO_ERR;
    inst_type_t type = INST_IGNORE;
    int len = 1;
    int i;

    for (i = 0; i < ARRAY_SIZE(relo_len); i++) {
        if ((inst & masks[i]) == types[i]) {
            type = types[i];
            len = relo_len[i];
            break;
        }
    }

    switch (type) {
    case INST_B:
    case INST_BC:
    case INST_BL:
        rc = relo_b(hook, inst_addr, inst, type);
        break;
    case INST_ADR:
    case INST_ADRP:
        rc = relo_adr(hook, inst_addr, inst, type);
        break;
    case INST_LDR_32:
    case INST_LDR_64:
    case INST_LDRSW_LIT:
    case INST_PRFM_LIT:
    case INST_LDR_SIMD_32:
    case INST_LDR_SIMD_64:
    case INST_LDR_SIMD_128:
        rc = relo_ldr(hook, inst_addr, inst, type);
        break;
    case INST_CBZ:
    case INST_CBNZ:
        rc = relo_cb(hook, inst_addr, inst);
        break;
    case INST_TBZ:
    case INST_TBNZ:
        rc = relo_tb(hook, inst_addr, inst);
        break;
    default:
        rc = relo_ignore(hook, inst);
        break;
    }

    hook->relo_insts_num += len;
    return rc;
}

hook_err_t hook_prepare(hook_t *hook)
{
    int i;

    if (is_bad_address((void *)hook->func_addr) || is_bad_address((void *)hook->origin_addr) ||
        is_bad_address((void *)hook->replace_addr) || is_bad_address((void *)hook->relo_addr))
        return -HOOK_BAD_ADDRESS;

    for (i = 0; i < TRAMPOLINE_MAX_NUM; i++)
        hook->origin_insts[i] = *((u32 *)hook->origin_addr + i);

    if (hook->origin_insts[0] == ARM64_PACIASP || hook->origin_insts[0] == ARM64_PACIBSP) {
        hook->tramp_insts[0] = ARM64_BTI_JC;
        hook->tramp_insts_num = 1 + branch_from_to(&hook->tramp_insts[1], hook->origin_addr, hook->replace_addr);
    } else {
        hook->tramp_insts_num = branch_from_to(hook->tramp_insts, hook->origin_addr, hook->replace_addr);
    }

    for (i = 0; i < ARRAY_SIZE(hook->relo_insts); i++)
        hook->relo_insts[i] = ARM64_NOP;
    hook->relo_insts_num = 0;

    for (i = 0; i < hook->tramp_insts_num; i++) {
        hook_err_t err = relocate_inst(hook, hook->origin_addr + i * 4, hook->origin_insts[i]);

        if (err)
            return -HOOK_BAD_RELO;
    }

    hook->relo_insts_num += branch_from_to(hook->relo_insts + hook->relo_insts_num,
                                           hook->relo_addr + hook->relo_insts_num * 4,
                                           hook->origin_addr + hook->tramp_insts_num * 4);

    flush_icache_range(hook->relo_addr, hook->relo_addr + sizeof(hook->relo_insts));
    return HOOK_NO_ERR;
}

void hook_install(hook_t *hook)
{
    void *addrs[TRAMPOLINE_MAX_NUM];
    int i;

    for (i = 0; i < hook->tramp_insts_num; i++)
        addrs[i] = (u32 *)hook->origin_addr + i;

    patch_text_array(addrs, hook->tramp_insts, hook->tramp_insts_num);
}

void hook_uninstall(hook_t *hook)
{
    void *addrs[TRAMPOLINE_MAX_NUM];
    int i;

    for (i = 0; i < hook->tramp_insts_num; i++)
        addrs[i] = (u32 *)hook->origin_addr + i;

    patch_text_array(addrs, hook->origin_insts, hook->tramp_insts_num);
}

typedef u64 (*transit0_func_t)(void);
typedef u64 (*transit4_func_t)(u64, u64, u64, u64);
typedef u64 (*transit8_func_t)(u64, u64, u64, u64, u64, u64, u64, u64);
typedef u64 (*transit12_func_t)(u64, u64, u64, u64, u64, u64, u64, u64, u64, u64, u64, u64);

static noinline u64 kp_transit0_template(void) __attribute__((section(".kp.transit0")));
static noinline u64 kp_transit0_template(void)
{
    u64 this_va;
    u32 *vptr;
    hook_chain_t *hook_chain;
    hook_fargs0_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (hook_chain_t *)((char *)vptr - offsetof(hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.chain = hook_chain;
    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain0_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit0_func_t origin_func = (transit0_func_t)hook_chain->hook.relo_addr;

        fargs.ret = origin_func();
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain0_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_transit0_template_end[];
asm(".pushsection .kp.transit0,\"ax\"\n"
    ".global kp_transit0_template_end\n"
    "kp_transit0_template_end:\n"
    ".popsection\n");

static noinline u64 kp_transit4_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3)
    __attribute__((section(".kp.transit4")));
static noinline u64 kp_transit4_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3)
{
    u64 this_va;
    u32 *vptr;
    hook_chain_t *hook_chain;
    hook_fargs4_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (hook_chain_t *)((char *)vptr - offsetof(hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.arg0 = arg0;
    fargs.arg1 = arg1;
    fargs.arg2 = arg2;
    fargs.arg3 = arg3;
    fargs.chain = hook_chain;

    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain4_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit4_func_t origin_func = (transit4_func_t)hook_chain->hook.relo_addr;

        fargs.ret = origin_func(fargs.arg0, fargs.arg1, fargs.arg2, fargs.arg3);
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain4_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_transit4_template_end[];
asm(".pushsection .kp.transit4,\"ax\"\n"
    ".global kp_transit4_template_end\n"
    "kp_transit4_template_end:\n"
    ".popsection\n");

static noinline u64 kp_transit8_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                         u64 arg7) __attribute__((section(".kp.transit8")));
static noinline u64 kp_transit8_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                         u64 arg7)
{
    u64 this_va;
    u32 *vptr;
    hook_chain_t *hook_chain;
    hook_fargs8_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (hook_chain_t *)((char *)vptr - offsetof(hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.arg0 = arg0;
    fargs.arg1 = arg1;
    fargs.arg2 = arg2;
    fargs.arg3 = arg3;
    fargs.arg4 = arg4;
    fargs.arg5 = arg5;
    fargs.arg6 = arg6;
    fargs.arg7 = arg7;
    fargs.chain = hook_chain;

    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain8_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit8_func_t origin_func = (transit8_func_t)hook_chain->hook.relo_addr;

        fargs.ret = origin_func(fargs.arg0, fargs.arg1, fargs.arg2, fargs.arg3,
                                fargs.arg4, fargs.arg5, fargs.arg6, fargs.arg7);
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain8_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_transit8_template_end[];
asm(".pushsection .kp.transit8,\"ax\"\n"
    ".global kp_transit8_template_end\n"
    "kp_transit8_template_end:\n"
    ".popsection\n");

static noinline u64 kp_transit12_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                          u64 arg7, u64 arg8, u64 arg9, u64 arg10, u64 arg11)
    __attribute__((section(".kp.transit12")));
static noinline u64 kp_transit12_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                          u64 arg7, u64 arg8, u64 arg9, u64 arg10, u64 arg11)
{
    u64 this_va;
    u32 *vptr;
    hook_chain_t *hook_chain;
    hook_fargs12_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (hook_chain_t *)((char *)vptr - offsetof(hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.arg0 = arg0;
    fargs.arg1 = arg1;
    fargs.arg2 = arg2;
    fargs.arg3 = arg3;
    fargs.arg4 = arg4;
    fargs.arg5 = arg5;
    fargs.arg6 = arg6;
    fargs.arg7 = arg7;
    fargs.arg8 = arg8;
    fargs.arg9 = arg9;
    fargs.arg10 = arg10;
    fargs.arg11 = arg11;
    fargs.chain = hook_chain;

    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain12_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit12_func_t origin_func = (transit12_func_t)hook_chain->hook.relo_addr;

        fargs.ret = origin_func(fargs.arg0, fargs.arg1, fargs.arg2, fargs.arg3,
                                fargs.arg4, fargs.arg5, fargs.arg6, fargs.arg7,
                                fargs.arg8, fargs.arg9, fargs.arg10, fargs.arg11);
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain12_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_transit12_template_end[];
asm(".pushsection .kp.transit12,\"ax\"\n"
    ".global kp_transit12_template_end\n"
    "kp_transit12_template_end:\n"
    ".popsection\n");

static noinline u64 kp_fp_transit0_template(void) __attribute__((section(".kp.fp.transit0")));
static noinline u64 kp_fp_transit0_template(void)
{
    u64 this_va;
    u32 *vptr;
    fp_hook_chain_t *hook_chain;
    hook_fargs0_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (fp_hook_chain_t *)((char *)vptr - offsetof(fp_hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.chain = hook_chain;
    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain0_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit0_func_t origin_func = (transit0_func_t)hook_chain->hook.origin_fp;

        fargs.ret = origin_func();
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain0_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_fp_transit0_template_end[];
asm(".pushsection .kp.fp.transit0,\"ax\"\n"
    ".global kp_fp_transit0_template_end\n"
    "kp_fp_transit0_template_end:\n"
    ".popsection\n");

static noinline u64 kp_fp_transit4_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3)
    __attribute__((section(".kp.fp.transit4")));
static noinline u64 kp_fp_transit4_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3)
{
    u64 this_va;
    u32 *vptr;
    fp_hook_chain_t *hook_chain;
    hook_fargs4_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (fp_hook_chain_t *)((char *)vptr - offsetof(fp_hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.arg0 = arg0;
    fargs.arg1 = arg1;
    fargs.arg2 = arg2;
    fargs.arg3 = arg3;
    fargs.chain = hook_chain;

    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain4_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit4_func_t origin_func = (transit4_func_t)hook_chain->hook.origin_fp;

        fargs.ret = origin_func(fargs.arg0, fargs.arg1, fargs.arg2, fargs.arg3);
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain4_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_fp_transit4_template_end[];
asm(".pushsection .kp.fp.transit4,\"ax\"\n"
    ".global kp_fp_transit4_template_end\n"
    "kp_fp_transit4_template_end:\n"
    ".popsection\n");

static noinline u64 kp_fp_transit8_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                            u64 arg7) __attribute__((section(".kp.fp.transit8")));
static noinline u64 kp_fp_transit8_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                            u64 arg7)
{
    u64 this_va;
    u32 *vptr;
    fp_hook_chain_t *hook_chain;
    hook_fargs8_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (fp_hook_chain_t *)((char *)vptr - offsetof(fp_hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.arg0 = arg0;
    fargs.arg1 = arg1;
    fargs.arg2 = arg2;
    fargs.arg3 = arg3;
    fargs.arg4 = arg4;
    fargs.arg5 = arg5;
    fargs.arg6 = arg6;
    fargs.arg7 = arg7;
    fargs.chain = hook_chain;

    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain8_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit8_func_t origin_func = (transit8_func_t)hook_chain->hook.origin_fp;

        fargs.ret = origin_func(fargs.arg0, fargs.arg1, fargs.arg2, fargs.arg3,
                                fargs.arg4, fargs.arg5, fargs.arg6, fargs.arg7);
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain8_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_fp_transit8_template_end[];
asm(".pushsection .kp.fp.transit8,\"ax\"\n"
    ".global kp_fp_transit8_template_end\n"
    "kp_fp_transit8_template_end:\n"
    ".popsection\n");

static noinline u64 kp_fp_transit12_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                             u64 arg7, u64 arg8, u64 arg9, u64 arg10, u64 arg11)
    __attribute__((section(".kp.fp.transit12")));
static noinline u64 kp_fp_transit12_template(u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6,
                                             u64 arg7, u64 arg8, u64 arg9, u64 arg10, u64 arg11)
{
    u64 this_va;
    u32 *vptr;
    fp_hook_chain_t *hook_chain;
    hook_fargs12_t fargs;
    int i;

    asm volatile("adr %0, ." : "=r"(this_va));
    vptr = (u32 *)this_va;
    while (*--vptr != ARM64_NOP)
        ;
    vptr--;
    hook_chain = (fp_hook_chain_t *)((char *)vptr - offsetof(fp_hook_chain_t, transit));

    fargs.skip_origin = 0;
    fargs.ret = 0;
    fargs.arg0 = arg0;
    fargs.arg1 = arg1;
    fargs.arg2 = arg2;
    fargs.arg3 = arg3;
    fargs.arg4 = arg4;
    fargs.arg5 = arg5;
    fargs.arg6 = arg6;
    fargs.arg7 = arg7;
    fargs.arg8 = arg8;
    fargs.arg9 = arg9;
    fargs.arg10 = arg10;
    fargs.arg11 = arg11;
    fargs.chain = hook_chain;

    for (i = 0; i < hook_chain->chain_items_max; i++) {
        hook_chain12_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->befores[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    if (!fargs.skip_origin) {
        transit12_func_t origin_func = (transit12_func_t)hook_chain->hook.origin_fp;

        fargs.ret = origin_func(fargs.arg0, fargs.arg1, fargs.arg2, fargs.arg3,
                                fargs.arg4, fargs.arg5, fargs.arg6, fargs.arg7,
                                fargs.arg8, fargs.arg9, fargs.arg10, fargs.arg11);
    }

    for (i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        hook_chain12_callback func;

        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        func = hook_chain->afters[i];
        if (func)
            func(&fargs, hook_chain->udata[i]);
    }

    return fargs.ret;
}

extern char kp_fp_transit12_template_end[];
asm(".pushsection .kp.fp.transit12,\"ax\"\n"
    ".global kp_fp_transit12_template_end\n"
    "kp_fp_transit12_template_end:\n"
    ".popsection\n");

static hook_err_t copy_transit_template(u32 *transit, int argno, bool fp_chain)
{
    const void *start;
    const void *end;
    size_t size;

    if (!fp_chain) {
        switch (argno) {
        case 0:
            start = kp_transit0_template;
            end = kp_transit0_template_end;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            start = kp_transit4_template;
            end = kp_transit4_template_end;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            start = kp_transit8_template;
            end = kp_transit8_template_end;
            break;
        default:
            start = kp_transit12_template;
            end = kp_transit12_template_end;
            break;
        }
    } else {
        switch (argno) {
        case 0:
            start = kp_fp_transit0_template;
            end = kp_fp_transit0_template_end;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            start = kp_fp_transit4_template;
            end = kp_fp_transit4_template_end;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            start = kp_fp_transit8_template;
            end = kp_fp_transit8_template_end;
            break;
        default:
            start = kp_fp_transit12_template;
            end = kp_fp_transit12_template_end;
            break;
        }
    }

    size = (uintptr_t)end - (uintptr_t)start;
    if ((size / sizeof(u32)) + 2 > TRANSIT_INST_NUM) {
        pr_err("kp_hook: transit template too large, argno=%d fp=%d size=%zu inst=%zu limit=%u\n",
               argno, fp_chain, size, size / sizeof(u32), TRANSIT_INST_NUM);
        return -HOOK_TRANSIT_NO_MEM;
    }

    memset(transit, 0, sizeof(u32) * TRANSIT_INST_NUM);
    transit[0] = ARM64_BTI_JC;
    transit[1] = ARM64_NOP;
    memcpy(&transit[2], start, size);
    flush_icache_range((unsigned long)transit, (unsigned long)transit + TRANSIT_INST_NUM * sizeof(u32));
    return HOOK_NO_ERR;
}

hook_err_t hook_chain_add(hook_chain_t *chain, void *before, void *after, void *udata)
{
    int i;

    for (i = 0; i < HOOK_CHAIN_NUM; i++) {
        if ((before && chain->befores[i] == before) || (after && chain->afters[i] == after))
            return -HOOK_DUPLICATED;

        if (chain->states[i] == CHAIN_ITEM_STATE_EMPTY) {
            chain->states[i] = CHAIN_ITEM_STATE_BUSY;
            dsb_ish();
            chain->udata[i] = udata;
            chain->befores[i] = before;
            chain->afters[i] = after;
            if (i + 1 > chain->chain_items_max)
                chain->chain_items_max = i + 1;
            dsb_ish();
            chain->states[i] = CHAIN_ITEM_STATE_READY;
            return HOOK_NO_ERR;
        }
    }

    return -HOOK_CHAIN_FULL;
}

void hook_chain_remove(hook_chain_t *chain, void *before, void *after)
{
    int i;

    for (i = 0; i < HOOK_CHAIN_NUM; i++) {
        if (chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        if ((before && chain->befores[i] == before) || (after && chain->afters[i] == after)) {
            chain->states[i] = CHAIN_ITEM_STATE_BUSY;
            dsb_ish();
            chain->udata[i] = NULL;
            chain->befores[i] = NULL;
            chain->afters[i] = NULL;
            dsb_ish();
            chain->states[i] = CHAIN_ITEM_STATE_EMPTY;
            return;
        }
    }
}

hook_err_t hook(void *func, void *replace, void **backup)
{
    hook_t *ctx;
    size_t alloc_size = 0;
    hook_err_t err;
    u64 origin;
    int rc;

    if (!g_runtime_ready)
        return -HOOK_INIT_FAILED;
    if (!func || !replace || !backup)
        return -HOOK_BAD_ADDRESS;

    origin = branch_func_addr((u64)func);

    mutex_lock(&g_hook_lock);
    if (find_entry_locked(origin)) {
        mutex_unlock(&g_hook_lock);
        return -HOOK_DUPLICATED;
    }

    ctx = alloc_exec(sizeof(*ctx), &alloc_size);
    if (!ctx) {
        mutex_unlock(&g_hook_lock);
        return -HOOK_NO_MEM;
    }

    ctx->func_addr = (u64)func;
    ctx->origin_addr = origin;
    ctx->replace_addr = (u64)replace;
    ctx->relo_addr = (u64)ctx->relo_insts;
    *backup = (void *)ctx->relo_addr;

    err = hook_prepare(ctx);
    if (!err)
        hook_install(ctx);

    if (!err) {
        rc = add_entry_locked(INLINE, origin, ctx, alloc_size);
        if (rc) {
            hook_uninstall(ctx);
            err = -HOOK_NO_MEM;
        }
    }

    if (err)
        free_exec(ctx);
    mutex_unlock(&g_hook_lock);
    return err;
}

void unhook(void *func)
{
    u64 origin;
    struct hook_entry *entry;

    if (!func)
        return;

    origin = branch_func_addr((u64)func);
    mutex_lock(&g_hook_lock);
    entry = find_entry_locked(origin);
    if (!entry || entry->type != INLINE) {
        mutex_unlock(&g_hook_lock);
        return;
    }

    hook_uninstall((hook_t *)entry->ctx);
    free_exec(entry->ctx);
    del_entry_locked(entry);
    mutex_unlock(&g_hook_lock);
}

hook_err_t hook_wrap(void *func, s32 argno, void *before, void *after, void *udata)
{
    hook_chain_t *chain;
    struct hook_entry *entry;
    size_t alloc_size = 0;
    hook_err_t err;
    int rc;
    u64 origin;

    if (!g_runtime_ready)
        return -HOOK_INIT_FAILED;
    if (is_bad_address(func))
        return -HOOK_BAD_ADDRESS;

    origin = branch_func_addr((u64)func);

    mutex_lock(&g_hook_lock);
    entry = find_entry_locked(origin);
    if (entry) {
        if (entry->type != INLINE_CHAIN) {
            mutex_unlock(&g_hook_lock);
            return -HOOK_DUPLICATED;
        }
        err = hook_chain_add((hook_chain_t *)entry->ctx, before, after, udata);
        mutex_unlock(&g_hook_lock);
        return err;
    }

    chain = alloc_exec(sizeof(*chain), &alloc_size);
    if (!chain) {
        mutex_unlock(&g_hook_lock);
        return -HOOK_NO_MEM;
    }

    chain->hook.func_addr = (u64)func;
    chain->hook.origin_addr = origin;
    chain->hook.replace_addr = (u64)chain->transit;
    chain->hook.relo_addr = (u64)chain->hook.relo_insts;

    err = hook_prepare(&chain->hook);
    if (err)
        goto out_free;

    err = copy_transit_template(chain->transit, argno, false);
    if (err)
        goto out_free;

    err = hook_chain_add(chain, before, after, udata);
    if (err)
        goto out_free;

    hook_install(&chain->hook);

    rc = add_entry_locked(INLINE_CHAIN, origin, chain, alloc_size);
    if (rc) {
        hook_uninstall(&chain->hook);
        err = -HOOK_NO_MEM;
        goto out_free;
    }

    mutex_unlock(&g_hook_lock);
    return HOOK_NO_ERR;

out_free:
    free_exec(chain);
    mutex_unlock(&g_hook_lock);
    return err;
}

void hook_unwrap_remove(void *func, void *before, void *after, int remove)
{
    u64 origin;
    struct hook_entry *entry;
    hook_chain_t *chain;
    int i;

    if (is_bad_address(func))
        return;

    origin = branch_func_addr((u64)func);
    mutex_lock(&g_hook_lock);
    entry = find_entry_locked(origin);
    if (!entry || entry->type != INLINE_CHAIN) {
        mutex_unlock(&g_hook_lock);
        return;
    }

    chain = (hook_chain_t *)entry->ctx;
    hook_chain_remove(chain, before, after);
    if (!remove) {
        mutex_unlock(&g_hook_lock);
        return;
    }

    for (i = 0; i < HOOK_CHAIN_NUM; i++) {
        if (chain->states[i] != CHAIN_ITEM_STATE_EMPTY) {
            mutex_unlock(&g_hook_lock);
            return;
        }
    }

    hook_uninstall(&chain->hook);
    free_exec(chain);
    del_entry_locked(entry);
    mutex_unlock(&g_hook_lock);
}

void fp_hook(uintptr_t fp_addr, void *replace, void **backup)
{
    if (!fp_addr || !replace || !backup)
        return;

    *backup = (void *)READ_ONCE(*(uintptr_t *)fp_addr);
    WRITE_ONCE(*(uintptr_t *)fp_addr, (uintptr_t)replace);
    dsb_ish();
    isb_sy();
}

void fp_unhook(uintptr_t fp_addr, void *backup)
{
    if (!fp_addr)
        return;

    WRITE_ONCE(*(uintptr_t *)fp_addr, (uintptr_t)backup);
    dsb_ish();
    isb_sy();
}

hook_err_t fp_hook_wrap(uintptr_t fp_addr, s32 argno, void *before, void *after, void *udata)
{
    fp_hook_chain_t *chain;
    struct hook_entry *entry;
    size_t alloc_size = 0;
    hook_err_t err;
    int i;

    if (!g_runtime_ready)
        return -HOOK_INIT_FAILED;
    if (is_bad_address((void *)fp_addr))
        return -HOOK_BAD_ADDRESS;

    mutex_lock(&g_hook_lock);
    entry = find_entry_locked(fp_addr);
    if (!entry) {
        chain = alloc_exec(sizeof(*chain), &alloc_size);
        if (!chain) {
            mutex_unlock(&g_hook_lock);
            return -HOOK_NO_MEM;
        }

        chain->hook.fp_addr = fp_addr;
        chain->hook.replace_addr = (u64)chain->transit;
        err = copy_transit_template(chain->transit, argno, true);
        if (err) {
            free_exec(chain);
            mutex_unlock(&g_hook_lock);
            return err;
        }

        fp_hook(fp_addr, (void *)chain->hook.replace_addr, (void **)&chain->hook.origin_fp);
        if (add_entry_locked(FUNCTION_POINTER_CHAIN, fp_addr, chain, alloc_size)) {
            fp_unhook(fp_addr, (void *)chain->hook.origin_fp);
            free_exec(chain);
            mutex_unlock(&g_hook_lock);
            return -HOOK_NO_MEM;
        }
        entry = find_entry_locked(fp_addr);
    } else if (entry->type != FUNCTION_POINTER_CHAIN) {
        mutex_unlock(&g_hook_lock);
        return -HOOK_DUPLICATED;
    }

    chain = (fp_hook_chain_t *)entry->ctx;
    for (i = 0; i < FP_HOOK_CHAIN_NUM; i++) {
        if ((before && chain->befores[i] == before) || (after && chain->afters[i] == after)) {
            mutex_unlock(&g_hook_lock);
            return -HOOK_DUPLICATED;
        }
        if (chain->states[i] == CHAIN_ITEM_STATE_EMPTY) {
            chain->states[i] = CHAIN_ITEM_STATE_BUSY;
            dsb_ish();
            chain->udata[i] = udata;
            chain->befores[i] = before;
            chain->afters[i] = after;
            if (i + 1 > chain->chain_items_max)
                chain->chain_items_max = i + 1;
            dsb_ish();
            chain->states[i] = CHAIN_ITEM_STATE_READY;
            mutex_unlock(&g_hook_lock);
            return HOOK_NO_ERR;
        }
    }

    mutex_unlock(&g_hook_lock);
    return -HOOK_CHAIN_FULL;
}

void fp_hook_unwrap(uintptr_t fp_addr, void *before, void *after)
{
    struct hook_entry *entry;
    fp_hook_chain_t *chain;
    int i;

    if (is_bad_address((void *)fp_addr))
        return;

    mutex_lock(&g_hook_lock);
    entry = find_entry_locked(fp_addr);
    if (!entry || entry->type != FUNCTION_POINTER_CHAIN) {
        mutex_unlock(&g_hook_lock);
        return;
    }

    chain = (fp_hook_chain_t *)entry->ctx;
    for (i = 0; i < FP_HOOK_CHAIN_NUM; i++) {
        if (chain->states[i] != CHAIN_ITEM_STATE_READY)
            continue;
        if ((before && chain->befores[i] == before) || (after && chain->afters[i] == after)) {
            chain->states[i] = CHAIN_ITEM_STATE_BUSY;
            dsb_ish();
            chain->udata[i] = NULL;
            chain->befores[i] = NULL;
            chain->afters[i] = NULL;
            dsb_ish();
            chain->states[i] = CHAIN_ITEM_STATE_EMPTY;
            break;
        }
    }

    for (i = 0; i < FP_HOOK_CHAIN_NUM; i++) {
        if (chain->states[i] != CHAIN_ITEM_STATE_EMPTY) {
            mutex_unlock(&g_hook_lock);
            return;
        }
    }

    fp_unhook(chain->hook.fp_addr, (void *)chain->hook.origin_fp);
    free_exec(chain);
    del_entry_locked(entry);
    mutex_unlock(&g_hook_lock);
}

int kp_hook_runtime_init(void)
{
    if (g_runtime_ready)
        return 0;

    memset(&g_syms, 0, sizeof(g_syms));
    g_syms.module_alloc = (module_alloc_t)kp_lookup_name("module_alloc");
    g_syms.module_memfree = (module_memfree_t)kp_lookup_name("module_memfree");
    g_syms.set_memory_x = (set_memory_x_t)kp_lookup_name("set_memory_x");
    g_syms.patch_text = (aarch64_insn_patch_text_t)kp_lookup_name("aarch64_insn_patch_text");
    g_syms.patch_text_nosync = (aarch64_insn_patch_text_nosync_t)kp_lookup_name("aarch64_insn_patch_text_nosync");

    if (!g_syms.module_alloc) {
        pr_err("kp_hook: failed to resolve module_alloc\n");
        return -ENOENT;
    }
    if (!g_syms.set_memory_x)
        pr_warn("kp_hook: failed to resolve set_memory_x\n");

    if (!g_syms.patch_text && !g_syms.patch_text_nosync)
        pr_warn("kp_hook: text patch helpers not found, using direct write fallback\n");

    g_runtime_ready = true;
    pr_info("kp_hook: runtime ready\n");
    return 0;
}

void kp_hook_runtime_exit(void)
{
    struct hook_entry *entry;
    struct hook_entry *tmp;

    mutex_lock(&g_hook_lock);
    list_for_each_entry_safe(entry, tmp, &g_hook_entries, node) {
        switch (entry->type) {
        case INLINE:
            hook_uninstall((hook_t *)entry->ctx);
            break;
        case INLINE_CHAIN:
            hook_uninstall(&((hook_chain_t *)entry->ctx)->hook);
            break;
        case FUNCTION_POINTER_CHAIN:
            fp_unhook(((fp_hook_chain_t *)entry->ctx)->hook.fp_addr,
                      (void *)((fp_hook_chain_t *)entry->ctx)->hook.origin_fp);
            break;
        default:
            break;
        }

        free_exec(entry->ctx);
        del_entry_locked(entry);
    }
    mutex_unlock(&g_hook_lock);

    g_runtime_ready = false;
}
