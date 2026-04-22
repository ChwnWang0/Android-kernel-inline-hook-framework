#ifndef KP_HOOK_H
#define KP_HOOK_H

#include <linux/types.h>
#include <linux/module.h>
#include <linux/hashtable.h>

#define INSTRUCTION_SIZE 4
#define HIJACK_INST_NUM 6
#define HIJACK_SIZE (INSTRUCTION_SIZE * HIJACK_INST_NUM)
#define HOOK_TARGET_OFFSET 0
#define CODE_SPACE_OFFSET 0

#define DEFAULT_HASH_BUCKET_BITS 10

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

int kp_hook_runtime_init(void);
void kp_hook_runtime_exit(void);

int hijack_target_prepare(void *target, void *hook_dest,
    void *hook_template_code_space, void *hook_func, char *mod_name);
int hijack_target_enable(void *target);
int hijack_target_disable(void *target, bool need_remove);
void hijack_target_disable_all(bool need_remove, char *mod_name);

#endif
