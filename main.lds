SECTIONS {
	__patchable_function_entries : { *(__patchable_function_entries) }
	.text : {
		init_module.cfi_jt = . + 0x2600;
		hook_add.cfi_jt = . + 0x2608;
		add.cfi_jt = . + 0x2610;
		do_hijack_target.cfi_jt = . + 0x2618;
		handler_panic_pre.cfi_jt = . + 0x2620;
		ret_handler_dump_backtrace.cfi_jt = . + 0x2628;
		cleanup_module.cfi_jt = . + 0x2630;
	}
}
