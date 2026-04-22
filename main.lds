SECTIONS {
	__patchable_function_entries : { *(__patchable_function_entries) }
	.text : {
		init_module.cfi_jt = . + 0x17a0;
		hook_add.cfi_jt = . + 0x17a8;
		add.cfi_jt = . + 0x17b0;
		do_hijack_target.cfi_jt = . + 0x17b8;
		handler_panic_pre.cfi_jt = . + 0x17c0;
		ret_handler_dump_backtrace.cfi_jt = . + 0x17c8;
		cleanup_module.cfi_jt = . + 0x17d0;
	}
}
