#ifndef BYPASS_H
#define BYPASS_H

/* 安全保护绕过函数 */

/* 绕过 CFI (控制流完整性) 检查 */
int bypass_cfi(void);

/* 禁用 kprobe 黑名单，允许 hook 受限函数 */
int disable_kprobe_blacklist(void);

#endif
