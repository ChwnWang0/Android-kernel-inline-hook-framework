#ifndef KP_HOOK_H
#define KP_HOOK_H

#include <linux/types.h>

#define HOOK_INTO_BRANCH_FUNC

typedef enum {
    HOOK_NO_ERR = 0,
    HOOK_BAD_ADDRESS = 4095,
    HOOK_DUPLICATED = 4094,
    HOOK_NO_MEM = 4093,
    HOOK_BAD_RELO = 4092,
    HOOK_TRANSIT_NO_MEM = 4091,
    HOOK_CHAIN_FULL = 4090,
    HOOK_INIT_FAILED = 4089,
} hook_err_t;

enum hook_type {
    NONE = 0,
    INLINE,
    INLINE_CHAIN,
    FUNCTION_POINTER_CHAIN,
};

typedef s8 chain_item_state;

#define CHAIN_ITEM_STATE_EMPTY 0
#define CHAIN_ITEM_STATE_READY 1
#define CHAIN_ITEM_STATE_BUSY 2

#define HOOK_CHAIN_NUM 0x10
#define FP_HOOK_CHAIN_NUM 0x20
#define TRAMPOLINE_MAX_NUM 6
#define RELOCATE_INST_NUM (4 * 8 + 8 - 4)
#define TRANSIT_INST_NUM 0x200
#define HOOK_LOCAL_DATA_NUM 8

#define ARM64_NOP 0xd503201f
#define ARM64_BTI_C 0xd503245f
#define ARM64_BTI_J 0xd503249f
#define ARM64_BTI_JC 0xd50324df
#define ARM64_PACIASP 0xd503233f
#define ARM64_PACIBSP 0xd503237f

typedef struct {
    u64 func_addr;
    u64 origin_addr;
    u64 replace_addr;
    u64 relo_addr;
    s32 tramp_insts_num;
    s32 relo_insts_num;
    u32 origin_insts[TRAMPOLINE_MAX_NUM] __aligned(8);
    u32 tramp_insts[TRAMPOLINE_MAX_NUM] __aligned(8);
    u32 relo_insts[RELOCATE_INST_NUM] __aligned(8);
} hook_t __aligned(8);

struct _hook_chain;

typedef struct {
    union {
        struct {
            u64 data0;
            u64 data1;
            u64 data2;
            u64 data3;
            u64 data4;
            u64 data5;
            u64 data6;
            u64 data7;
        };
        u64 data[HOOK_LOCAL_DATA_NUM];
    };
} hook_local_t;

typedef struct {
    void *chain;
    int skip_origin;
    hook_local_t local;
    u64 ret;
    u64 args[0];
} hook_fargs0_t __aligned(8);

typedef struct {
    void *chain;
    int skip_origin;
    hook_local_t local;
    u64 ret;
    union {
        struct {
            u64 arg0;
            u64 arg1;
            u64 arg2;
            u64 arg3;
        };
        u64 args[4];
    };
} hook_fargs4_t __aligned(8);

typedef hook_fargs4_t hook_fargs1_t;
typedef hook_fargs4_t hook_fargs2_t;
typedef hook_fargs4_t hook_fargs3_t;

typedef struct {
    void *chain;
    int skip_origin;
    hook_local_t local;
    u64 ret;
    union {
        struct {
            u64 arg0;
            u64 arg1;
            u64 arg2;
            u64 arg3;
            u64 arg4;
            u64 arg5;
            u64 arg6;
            u64 arg7;
        };
        u64 args[8];
    };
} hook_fargs8_t __aligned(8);

typedef hook_fargs8_t hook_fargs5_t;
typedef hook_fargs8_t hook_fargs6_t;
typedef hook_fargs8_t hook_fargs7_t;

typedef struct {
    void *chain;
    int skip_origin;
    hook_local_t local;
    u64 ret;
    union {
        struct {
            u64 arg0;
            u64 arg1;
            u64 arg2;
            u64 arg3;
            u64 arg4;
            u64 arg5;
            u64 arg6;
            u64 arg7;
            u64 arg8;
            u64 arg9;
            u64 arg10;
            u64 arg11;
        };
        u64 args[12];
    };
} hook_fargs12_t __aligned(8);

typedef hook_fargs12_t hook_fargs9_t;
typedef hook_fargs12_t hook_fargs10_t;
typedef hook_fargs12_t hook_fargs11_t;

typedef void (*hook_chain0_callback)(hook_fargs0_t *fargs, void *udata);
typedef void (*hook_chain1_callback)(hook_fargs1_t *fargs, void *udata);
typedef void (*hook_chain2_callback)(hook_fargs2_t *fargs, void *udata);
typedef void (*hook_chain3_callback)(hook_fargs3_t *fargs, void *udata);
typedef void (*hook_chain4_callback)(hook_fargs4_t *fargs, void *udata);
typedef void (*hook_chain5_callback)(hook_fargs5_t *fargs, void *udata);
typedef void (*hook_chain6_callback)(hook_fargs6_t *fargs, void *udata);
typedef void (*hook_chain7_callback)(hook_fargs7_t *fargs, void *udata);
typedef void (*hook_chain8_callback)(hook_fargs8_t *fargs, void *udata);
typedef void (*hook_chain9_callback)(hook_fargs9_t *fargs, void *udata);
typedef void (*hook_chain10_callback)(hook_fargs10_t *fargs, void *udata);
typedef void (*hook_chain11_callback)(hook_fargs11_t *fargs, void *udata);
typedef void (*hook_chain12_callback)(hook_fargs12_t *fargs, void *udata);

typedef struct _hook_chain {
    hook_t hook;
    s32 chain_items_max;
    chain_item_state states[HOOK_CHAIN_NUM];
    void *udata[HOOK_CHAIN_NUM];
    void *befores[HOOK_CHAIN_NUM];
    void *afters[HOOK_CHAIN_NUM];
    u32 transit[TRANSIT_INST_NUM];
} hook_chain_t __aligned(8);

typedef struct {
    uintptr_t fp_addr;
    u64 replace_addr;
    u64 origin_fp;
} fp_hook_t __aligned(8);

typedef struct _fphook_chain {
    fp_hook_t hook;
    s32 chain_items_max;
    chain_item_state states[FP_HOOK_CHAIN_NUM];
    void *udata[FP_HOOK_CHAIN_NUM];
    void *befores[FP_HOOK_CHAIN_NUM];
    void *afters[FP_HOOK_CHAIN_NUM];
    u32 transit[TRANSIT_INST_NUM];
} fp_hook_chain_t __aligned(8);

int kp_hook_runtime_init(void);
void kp_hook_runtime_exit(void);

hook_err_t hook_prepare(hook_t *hook);
void hook_install(hook_t *hook);
void hook_uninstall(hook_t *hook);

hook_err_t hook(void *func, void *replace, void **backup);
void unhook(void *func);

hook_err_t hook_chain_add(hook_chain_t *chain, void *before, void *after, void *udata);
void hook_chain_remove(hook_chain_t *chain, void *before, void *after);
hook_err_t hook_wrap(void *func, s32 argno, void *before, void *after, void *udata);
void hook_unwrap_remove(void *func, void *before, void *after, int remove);

static inline void hook_unwrap(void *func, void *before, void *after)
{
    hook_unwrap_remove(func, before, after, 1);
}

static inline void *wrap_get_origin_func(void *hook_args)
{
    hook_fargs0_t *args = (hook_fargs0_t *)hook_args;
    hook_chain_t *chain = (hook_chain_t *)args->chain;
    return (void *)chain->hook.relo_addr;
}

void fp_hook(uintptr_t fp_addr, void *replace, void **backup);
void fp_unhook(uintptr_t fp_addr, void *backup);
hook_err_t fp_hook_wrap(uintptr_t fp_addr, s32 argno, void *before, void *after, void *udata);
void fp_hook_unwrap(uintptr_t fp_addr, void *before, void *after);

static inline void *fp_get_origin_func(void *hook_args)
{
    hook_fargs0_t *args = (hook_fargs0_t *)hook_args;
    fp_hook_chain_t *chain = (fp_hook_chain_t *)args->chain;
    return (void *)chain->hook.origin_fp;
}

static inline hook_err_t hook_wrap0(void *func, hook_chain0_callback before, hook_chain0_callback after, void *udata)
{
    return hook_wrap(func, 0, before, after, udata);
}

static inline hook_err_t hook_wrap1(void *func, hook_chain1_callback before, hook_chain1_callback after, void *udata)
{
    return hook_wrap(func, 1, before, after, udata);
}

static inline hook_err_t hook_wrap2(void *func, hook_chain2_callback before, hook_chain2_callback after, void *udata)
{
    return hook_wrap(func, 2, before, after, udata);
}

static inline hook_err_t hook_wrap3(void *func, hook_chain3_callback before, hook_chain3_callback after, void *udata)
{
    return hook_wrap(func, 3, before, after, udata);
}

static inline hook_err_t hook_wrap4(void *func, hook_chain4_callback before, hook_chain4_callback after, void *udata)
{
    return hook_wrap(func, 4, before, after, udata);
}

static inline hook_err_t hook_wrap5(void *func, hook_chain5_callback before, hook_chain5_callback after, void *udata)
{
    return hook_wrap(func, 5, before, after, udata);
}

static inline hook_err_t hook_wrap6(void *func, hook_chain6_callback before, hook_chain6_callback after, void *udata)
{
    return hook_wrap(func, 6, before, after, udata);
}

static inline hook_err_t hook_wrap7(void *func, hook_chain7_callback before, hook_chain7_callback after, void *udata)
{
    return hook_wrap(func, 7, before, after, udata);
}

static inline hook_err_t hook_wrap8(void *func, hook_chain8_callback before, hook_chain8_callback after, void *udata)
{
    return hook_wrap(func, 8, before, after, udata);
}

static inline hook_err_t hook_wrap9(void *func, hook_chain9_callback before, hook_chain9_callback after, void *udata)
{
    return hook_wrap(func, 9, before, after, udata);
}

static inline hook_err_t hook_wrap10(void *func, hook_chain10_callback before, hook_chain10_callback after, void *udata)
{
    return hook_wrap(func, 10, before, after, udata);
}

static inline hook_err_t hook_wrap11(void *func, hook_chain11_callback before, hook_chain11_callback after, void *udata)
{
    return hook_wrap(func, 11, before, after, udata);
}

static inline hook_err_t hook_wrap12(void *func, hook_chain12_callback before, hook_chain12_callback after, void *udata)
{
    return hook_wrap(func, 12, before, after, udata);
}

static inline hook_err_t fp_hook_wrap0(uintptr_t fp_addr, hook_chain0_callback before, hook_chain0_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 0, before, after, udata);
}

static inline hook_err_t fp_hook_wrap1(uintptr_t fp_addr, hook_chain1_callback before, hook_chain1_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 1, before, after, udata);
}

static inline hook_err_t fp_hook_wrap2(uintptr_t fp_addr, hook_chain2_callback before, hook_chain2_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 2, before, after, udata);
}

static inline hook_err_t fp_hook_wrap3(uintptr_t fp_addr, hook_chain3_callback before, hook_chain3_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 3, before, after, udata);
}

static inline hook_err_t fp_hook_wrap4(uintptr_t fp_addr, hook_chain4_callback before, hook_chain4_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 4, before, after, udata);
}

static inline hook_err_t fp_hook_wrap5(uintptr_t fp_addr, hook_chain5_callback before, hook_chain5_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 5, before, after, udata);
}

static inline hook_err_t fp_hook_wrap6(uintptr_t fp_addr, hook_chain6_callback before, hook_chain6_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 6, before, after, udata);
}

static inline hook_err_t fp_hook_wrap7(uintptr_t fp_addr, hook_chain7_callback before, hook_chain7_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 7, before, after, udata);
}

static inline hook_err_t fp_hook_wrap8(uintptr_t fp_addr, hook_chain8_callback before, hook_chain8_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 8, before, after, udata);
}

static inline hook_err_t fp_hook_wrap9(uintptr_t fp_addr, hook_chain9_callback before, hook_chain9_callback after,
                                       void *udata)
{
    return fp_hook_wrap(fp_addr, 9, before, after, udata);
}

static inline hook_err_t fp_hook_wrap10(uintptr_t fp_addr, hook_chain10_callback before, hook_chain10_callback after,
                                        void *udata)
{
    return fp_hook_wrap(fp_addr, 10, before, after, udata);
}

static inline hook_err_t fp_hook_wrap11(uintptr_t fp_addr, hook_chain11_callback before, hook_chain11_callback after,
                                        void *udata)
{
    return fp_hook_wrap(fp_addr, 11, before, after, udata);
}

static inline hook_err_t fp_hook_wrap12(uintptr_t fp_addr, hook_chain12_callback before, hook_chain12_callback after,
                                        void *udata)
{
    return fp_hook_wrap(fp_addr, 12, before, after, udata);
}

#endif
