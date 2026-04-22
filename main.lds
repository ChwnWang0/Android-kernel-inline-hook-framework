SECTIONS {
	__patchable_function_entries : { *(__patchable_function_entries) }
	.text : {
		init_module.cfi_jt = . + 0x13e8;
		before_add.cfi_jt = . + 0x13f0;
		after_add.cfi_jt = . + 0x13f8;
		add.cfi_jt = . + 0x1400;
		hook_dispatch_add.cfi_jt = . + 0x1408;
		handler_panic_pre.cfi_jt = . + 0x1410;
		ret_handler_dump_backtrace.cfi_jt = . + 0x1418;
		cleanup_module.cfi_jt = . + 0x1420;
	}
}
