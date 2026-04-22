SECTIONS {
	__patchable_function_entries : { *(__patchable_function_entries) }
	.text : {
		init_module.cfi_jt = . + 0x3258;
		add.cfi_jt = . + 0x3260;
		before_add.cfi_jt = . + 0x3268;
		after_add.cfi_jt = . + 0x3270;
		patch_request_cb.cfi_jt = . + 0x3278;
		handler_panic_pre.cfi_jt = . + 0x3280;
		ret_handler_dump_backtrace.cfi_jt = . + 0x3288;
		cleanup_module.cfi_jt = . + 0x3290;
		.kp.transit4.cfi_jt = . + 0x3298;
		.kp.fp.transit4.cfi_jt = . + 0x32a0;
		.kp.transit8.cfi_jt = . + 0x32a8;
		.kp.fp.transit8.cfi_jt = . + 0x32b0;
		.kp.transit12.cfi_jt = . + 0x32b8;
		.kp.fp.transit12.cfi_jt = . + 0x32c0;
		.kp.transit0.cfi_jt = . + 0x32c8;
		.kp.fp.transit0.cfi_jt = . + 0x32d0;
	}
}
