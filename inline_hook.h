#ifndef INLINE_HOOK_H
#define INLINE_HOOK_H

#include <linux/types.h>
#include <linux/module.h>
#include <linux/hashtable.h>

#define INSTRUCTION_SIZE 4
#define HIJACK_INST_NUM 6
#define HIJACK_SIZE (INSTRUCTION_SIZE * HIJACK_INST_NUM)
#define HOOK_TARGET_OFFSET 0
#define CODE_SPACE_OFFSET 0

#define DEFAULT_HASH_BUCKET_BITS 10

/* 重定位指令缓冲区最大长度 */
#define MAX_RELO_INSTS 64

/* ARM64 指令定义 */
#define ARM64_NOP 0xD503201F
#define ARM64_BTI_C 0xD503245F
#define ARM64_BTI_J 0xD503249F
#define ARM64_BTI_JC 0xD50324DF

#define INST_B 0x14000000
#define INST_BC 0x54000000
#define INST_BL 0x94000000
#define INST_ADR 0x10000000
#define INST_ADRP 0x90000000
#define INST_LDR_32 0x18000000
#define INST_LDR_64 0x58000000
#define INST_LDRSW_LIT 0x98000000
#define INST_CBZ 0x34000000
#define INST_CBNZ 0x35000000
#define INST_TBZ 0x36000000
#define INST_TBNZ 0x37000000

#define MASK_B 0xFC000000
#define MASK_BC 0xFF000010
#define MASK_BL 0xFC000000
#define MASK_ADR 0x9F000000
#define MASK_ADRP 0x9F000000
#define MASK_LDR_32 0xFF000000
#define MASK_LDR_64 0xFF000000
#define MASK_LDRSW_LIT 0xFF000000
#define MASK_CBZ 0x7F000000u
#define MASK_CBNZ 0x7F000000u
#define MASK_TBZ 0x7F000000u
#define MASK_TBNZ 0x7F000000u

#define jhash_pointer(pointer) jhash((&pointer), sizeof(pointer), 0x95279527)

struct sym_hook {
    void *target;
    void *hook_dest;
    void *template_return_addr;
    void *hook_template_code_space;
    void *hook_func;
    char *mod_name;
    bool enabled;
    struct hlist_node node;
    unsigned char target_code[HIJACK_SIZE];

    /* 重定位支持 */
    uint32_t origin_insts[HIJACK_INST_NUM];
    uint32_t relo_insts[MAX_RELO_INSTS];
    int relo_insts_num;
    int tramp_insts_num;
};

#define HOOK_FUNC_TEMPLATE(s) \
extern void hook_##s##_template(void); \
asm ( \
    ".globl hook_"#s"_template\n\t" \
    "hook_"#s"_template:\n\t" \
    "ldp x1, x0, [sp], #0x20\n\t" \
    "b hook_"#s"\n\t" \
    \
    ".globl "#s"_code_space\n\t" \
    #s"_code_space:\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
    ".word 0\n\t" \
)

#define GET_TEMPLATE_ADDERSS(s) \
({ \
    void *template; \
    __asm__ volatile ("ldr %0, =hook_"#s"_template\n\t" : "=r"(template)); \
    template; \
})

#define GET_CODESPACE_ADDERSS(s) \
({ \
    void *codespace; \
    __asm__ volatile ("ldr %0, ="#s"_code_space\n\t" : "=r"(codespace)); \
    codespace; \
})

#define GET_HOOK_FUNC_ADDRESS(s) \
({ \
    void *hook_func; \
    __asm__ volatile ("ldr %0, =hook_"#s"\n\t" : "=r"(hook_func)); \
    hook_func; \
})

#define HIJACK_TARGET_PREP_HOOK(addr, fn) \
    hijack_target_prepare(addr, GET_TEMPLATE_ADDERSS(fn), \
        GET_CODESPACE_ADDERSS(fn), GET_HOOK_FUNC_ADDRESS(fn), \
        module_name(THIS_MODULE))

#define HIJACK_TARGET_PREP_REPL(addr, fn) \
    hijack_target_prepare(addr, GET_TEMPLATE_ADDERSS(fn), \
        NULL, GET_HOOK_FUNC_ADDRESS(fn), \
        module_name(THIS_MODULE))

/* Inline hook 运行时 API */
int inline_hook_init(void);
void inline_hook_exit(void);

int hijack_target_prepare(void *target, void *hook_dest,
    void *hook_template_code_space, void *hook_func, char *mod_name);
int hijack_target_enable(void *target);
int hijack_target_disable(void *target, bool need_remove);
void hijack_target_disable_all(bool need_remove, char *mod_name);

#endif
