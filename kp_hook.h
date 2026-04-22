#ifndef KP_HOOK_H
#define KP_HOOK_H

#include <linux/types.h>

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

#define HOOK_LOCAL_DATA_NUM 8

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

typedef void (*hook_chain1_callback)(hook_fargs1_t *fargs, void *udata);
typedef void (*hook_chain2_callback)(hook_fargs2_t *fargs, void *udata);
typedef void (*hook_chain3_callback)(hook_fargs3_t *fargs, void *udata);

int kp_hook_runtime_init(void);
void kp_hook_runtime_exit(void);

hook_err_t hook_wrap(void *func, s32 argno, void *before, void *after, void *udata);
void hook_unwrap_remove(void *func, void *before, void *after, int remove);

static inline void hook_unwrap(void *func, void *before, void *after)
{
    hook_unwrap_remove(func, before, after, 1);
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

#endif
