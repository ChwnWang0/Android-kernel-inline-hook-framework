/*
 * @Author: 野生指针 <1703214369@qq.com>
 * @Date: 2026-04-21 23:31:18
 * @LastEditors: 野生指针 <1703214369@qq.com>
 * @LastEditTime: 2026-04-28 23:38:54
 * @FilePath: /inlinehooktest/bypass.h
 * @Description: 联系我QQ 1703214369
 * 
 * Copyright (c) 2026 by 野生指针 <1703214369@qq.com>, All Rights Reserved. 
 */
#ifndef BYPASS_H
#define BYPASS_H

/* 安全保护绕过函数 */

/* 绕过 CFI (控制流完整性) 检查 */
int bypass_cfi(void);

/* 禁用 kprobe 黑名单，允许 hook 受限函数 */
int disable_kprobe_blacklist(void);

#endif
