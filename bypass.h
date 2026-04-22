#ifndef BYPASS_H
#define BYPASS_H

/* Security protection bypass functions */

/* Bypass CFI (Control Flow Integrity) checks */
int bypass_cfi(void);

/* Disable kprobe blacklist to allow hooking restricted functions */
int disable_kprobe_blacklist(void);

#endif
