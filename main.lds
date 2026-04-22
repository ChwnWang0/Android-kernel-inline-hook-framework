SECTIONS {
	__patchable_function_entries : { *(__patchable_function_entries) }
	.text : {
		init_module.cfi_jt = . + 0x1740;
		hook_add.cfi_jt = . + 0x1748;
		add.cfi_jt = . + 0x1750;
		do_hijack_target.cfi_jt = . + 0x1758;
		handler_panic_pre.cfi_jt = . + 0x1760;
		ret_handler_dump_backtrace.cfi_jt = . + 0x1768;
		cleanup_module.cfi_jt = . + 0x1770;
	}
}
