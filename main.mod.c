#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x7c24b32d, "module_layout" },
	{ 0xdcb764ad, "memset" },
	{ 0x4829a47e, "memcpy" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x98a9d10c, "__stack_chk_fail" },
	{ 0xac594e02, "__cpu_online_mask" },
	{ 0x8900b200, "kmalloc_caches" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xe1537255, "__list_del_entry_valid" },
	{ 0x999e8297, "vfree" },
	{ 0x68f31cbd, "__list_add_valid" },
	{ 0xdfc04014, "stop_machine" },
	{ 0x2cd90edd, "__cfi_slowpath" },
	{ 0xa6e1a69d, "kick_all_cpus_sync" },
	{ 0xe2e0c7c6, "__flush_icache_range" },
	{ 0xb38391e9, "kmem_cache_alloc_trace" },
	{ 0xcd1b966b, "vfs_fsync" },
	{ 0x71316503, "kmsg_dump_get_line" },
	{ 0x616b30c7, "kmsg_dump_rewind" },
	{ 0x37a0cba, "kfree" },
	{ 0x18b23dc5, "unregister_kprobe" },
	{ 0x8ea54251, "unregister_kretprobe" },
	{ 0xc502eb6d, "register_kprobe" },
	{ 0x881262d4, "register_kretprobe" },
	{ 0xc5850110, "printk" },
	{ 0xe8b268ae, "mutex_unlock" },
	{ 0xeb9065d9, "mutex_lock" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "55B0865C033F6A8A44B89CF");
