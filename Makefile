obj-m += inlinehook.o
inlinehook-objs := main.o arch_helper.o bypass.o inline_hook.o hook_vfs_read_test.o
