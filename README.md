<p align="center">
  <h1 align="center">ARM64 Inline Hook Framework</h1>
  <p align="center">
    轻量级 ARM64 内核 inline hook 框架，支持完整指令重定位
    <br />
    <a href="#快速开始">快速开始</a> · <a href="#api-参考">API 参考</a> · <a href="#hook-执行流程">执行流程</a>
  </p>
</p>

---

## 简介

基于 [kernel-inline-hook-framework](https://github.com/AnyKernel3/kernel-inline-hook-framework) 和 [KernelPatch](https://github.com/bmax121/KernelPatch) 移植的 ARM64 内核 inline hook 框架。

可以 hook 包含任意 PC 相对指令的内核函数，自动完成指令重定位，无需手动处理。

## 特性

| 能力 | 说明 |
| :--- | :--- |
| 长跳转 | 6 指令跳板（stp/ldr/br + 8 字节地址 + ldp），覆盖 64 位全地址空间 |
| 指令重定位 | B / BL / B.cond / ADR / ADRP / LDR literal / CBZ / CBNZ / TBZ / TBNZ |
| BTI 兼容 | 正确处理 BTI C/J/JC 指令，不触发内核保护 |
| 跳转桩跟随 | 自动识别编译器生成的 `bti c + b trampoline` 桩 |
| 多核安全 | 通过 `stop_machine` 原子写入 |
| 栈活跃性检查 | 避免 hook 正在执行的函数 |
| 多 hook 管理 | 基于 hashtable，支持按模块名批量卸载 |
| CFI 绕过 | 自动 patch CFI 相关检查函数 |
| kprobe 黑名单禁用 | 允许 hook 受限内核函数 |

## 项目结构

```text
├── main.c                  # 模块入口、测试用例
├── inline_hook.h/c         # hook 框架核心（重定位、hijack、hashtable 管理）
├── arch_helper.h/c         # 架构辅助（kallsyms 符号查找、指令写入）
├── bypass.h/c              # 安全保护绕过（CFI bypass、kprobe blacklist）
├── hook_vfs_read_test.h/c  # vfs_read hook 示例
└── Makefile                # 内核模块编译配置
```

## 快速开始

### 编译

需要对应 Android 内核源码或交叉编译工具链：

```bash
sudo ./ddk_build.sh android12-5.10
```

> 支持：android12-5.10 / android13-5.15 / android14-6.1 / android15-6.6 / android16-6.12

### 加载模块

```bash
insmod inlinehook.ko    # 加载
rmmod inlinehook        # 卸载
```

## 使用方法

### 第一步：声明 hook 模板

```c
HOOK_FUNC_TEMPLATE(目标函数名);
```

展开后生成：
- `hook_xxx_template` — hook 入口跳板
- `xxx_code_space` — 重定位后原始指令的执行空间

### 第二步：编写 hook 函数

函数名必须为 `hook_` + 目标函数名，签名与原函数完全一致：

```c
static __nocfi __attribute__((used)) 返回类型 hook_目标函数名(参数列表)
{
    返回类型 (*origin)(参数列表);

    pr_info("hooked!\n");

    origin = (返回类型 (*)(参数列表))GET_CODESPACE_ADDERSS(目标函数名);
    return origin(参数);
}
```

### 第三步：安装 hook

```c
void *target = (void *)kallsyms_lookup_name_ex("目标函数名");

HIJACK_TARGET_PREP_HOOK(target, 目标函数名);   // 准备
hijack_target_enable(target);                   // 激活
```

### 第四步：卸载 hook

```c
hijack_target_disable(target, true);  // true = 从 hashtable 中移除
```

### 完整示例：hook vfs_read

```c
#include "inline_hook.h"
#include "arch_helper.h"

HOOK_FUNC_TEMPLATE(vfs_read);
static __nocfi __attribute__((used)) ssize_t hook_vfs_read(
    struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    ssize_t (*origin)(struct file *, char __user *, size_t, loff_t *);

    pr_info("vfs_read: reading %zu bytes\n", count);

    origin = (void *)GET_CODESPACE_ADDERSS(vfs_read);
    return origin(file, buf, count, pos);
}

static void *vfs_read_fn;

int my_hook_init(void)
{
    vfs_read_fn = (void *)kallsyms_lookup_name_ex("vfs_read");
    if (!vfs_read_fn)
        return -ENOENT;
    if (HIJACK_TARGET_PREP_HOOK(vfs_read_fn, vfs_read))
        return -EFAULT;
    return hijack_target_enable(vfs_read_fn);
}

void my_hook_exit(void)
{
    hijack_target_disable(vfs_read_fn, true);
}
```

## 命名约定

以 `vfs_read` 为例，三个名字必须保持一致：

| 宏 / 函数 | 传入的名字 | 展开后的符号 |
| :--- | :--- | :--- |
| `HOOK_FUNC_TEMPLATE(vfs_read)` | vfs_read | `hook_vfs_read_template`、`vfs_read_code_space` |
| `hook_vfs_read()` | — | 用户编写的 hook 函数 |
| `HIJACK_TARGET_PREP_HOOK(addr, vfs_read)` | vfs_read | 查找上述三个符号的地址 |

## Hook 执行流程

```text
目标函数入口
  │
  ▼
 [stp x1,x0] + [ldr x0, addr] + [br x0]          ← 6 指令长跳转，替换原始入口
  │
  ▼
 hook_xxx_template: [ldp x1,x0] + [b hook_xxx]    ← 恢复寄存器，跳到 hook
  │
  ▼
 hook_xxx()                                        ← 用户 hook 逻辑
  │
  ▼
 xxx_code_space:                                   ← 重定位后的原始指令
  ├── [原始 6 条指令的等价序列]
  └── [ldr x17, addr] + [br x17]                  ← 跳回目标函数 +24
        │
        ▼
      目标函数剩余部分继续执行
```

## 指令重定位

被 hook 的 6 条指令中若包含 PC 相对指令，直接复制会导致跳转目标错误。重定位机制将其转换为等价的绝对地址序列：

| 原始指令 | 重定位方式 | 指令数 |
| :--- | :--- | :---: |
| B / BL | LDR X17 + BR X17（BL 额外保存 X30） | 6-8 |
| B.cond | B.cond + LDR X17 + BR X17 | 8 |
| ADR / ADRP | LDR Xd, =addr | 4 |
| LDR literal | LDR Xt, =addr + LDR Xt, [Xt] | 6 |
| CBZ / CBNZ / TBZ / TBNZ | CBZ + LDR X17 + BR X17 | 8 |
| BTI C/J/JC | 直接复制 | 1 |
| 其他指令 | 直接复制 | 1 |

## API 参考

| 函数 | 说明 |
| :--- | :--- |
| `inline_hook_init()` | 初始化 hook 运行时（查找内核符号） |
| `inline_hook_exit()` | 卸载所有 hook 并清理 |
| `hijack_target_prepare(...)` | 准备 hook（分析指令、构建重定位、注册） |
| `hijack_target_enable(addr)` | 激活 hook |
| `hijack_target_disable(addr, remove)` | 禁用 hook，`remove=true` 时从 hashtable 删除 |
| `hijack_target_disable_all(remove, mod_name)` | 批量禁用，可按模块名过滤 |
| `kallsyms_lookup_name_ex(name)` | 查找内核符号地址 |
| `hook_write_range(target, source, size)` | 写入只读内存 |
| `bypass_cfi()` | 绕过 CFI 保护 |
| `disable_kprobe_blacklist()` | 禁用 kprobe 黑名单 |

## 依赖

- ARM64 架构
- 内核启用 `CONFIG_KPROBES`
- 内核包含 `aarch64_insn_write` 符号

## 作者

**野生指针**

| 联系方式 | |
| :--- | :--- |
| QQ | 1703214369 |
| 邮箱 | 1703214369@qq.com |
| 交流群 (QQ) | 1092055800 |

## License

[GPL-2.0](LICENSE)

## 致谢

- [kernel-inline-hook-framework](https://github.com/AnyKernel3/kernel-inline-hook-framework) — hijack 架构设计
- [KernelPatch](https://github.com/bmax121/KernelPatch) — 指令重定位实现参考
