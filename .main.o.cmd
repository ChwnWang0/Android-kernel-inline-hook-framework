cmd_/mnt/driver/main.o := clang -Wp,-MMD,/mnt/driver/.main.o.d -nostdinc -isystem /opt/ddk/clang/clang-r416183b/lib64/clang/12.0.5/include -I/opt/ddk/src/android12-5.10/arch/arm64/include -I./arch/arm64/include/generated -I/opt/ddk/src/android12-5.10/include -I./include -I/opt/ddk/src/android12-5.10/arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi -I/opt/ddk/src/android12-5.10/include/uapi -I./include/generated/uapi -include /opt/ddk/src/android12-5.10/include/linux/kconfig.h -include /opt/ddk/src/android12-5.10/include/linux/compiler_types.h -D__KERNEL__ --target=aarch64-linux-gnu --prefix=/usr/bin/aarch64-linux-gnu- --gcc-toolchain=/usr -fintegrated-as -Werror=unknown-warning-option -mlittle-endian -DKASAN_SHADOW_SCALE_SHIFT= -Qunused-arguments -fmacro-prefix-map=/opt/ddk/src/android12-5.10/= -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Wno-format-security -std=gnu89 -mgeneral-regs-only -DCONFIG_CC_HAS_K_CONSTRAINT=1 -Wno-psabi -fno-asynchronous-unwind-tables -fno-unwind-tables -mbranch-protection=pac-ret+leaf+bti -Wa,-march=armv8.5-a -DARM64_ASM_ARCH='"armv8.5-a"' -ffixed-x18 -DKASAN_SHADOW_SCALE_SHIFT= -fno-delete-null-pointer-checks -Wno-frame-address -Wno-address-of-packed-member -O2 -Wframe-larger-than=2048 -fstack-protector-strong -Werror -Wno-format-invalid-specifier -Wno-gnu -mno-global-merge -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang -g -gdwarf-4 -fsanitize=shadow-call-stack -flto -fvisibility=default -fsanitize=cfi -fsanitize-cfi-cross-dso -fno-sanitize-cfi-canonical-jump-tables -fno-sanitize-blacklist -Wdeclaration-after-statement -Wvla -Wno-pointer-sign -Wno-array-bounds -fno-strict-overflow -fno-stack-check -Werror=date-time -Werror=incompatible-pointer-types -fno-builtin-wcslen -Wno-initializer-overrides -Wno-format -Wno-sign-compare -Wno-format-zero-length -Wno-pointer-to-enum-cast -Wno-tautological-constant-out-of-range-compare -Wno-enum-compare-conditional  -fsanitize=array-bounds -fsanitize=local-bounds -fsanitize-undefined-trap-on-error  -DMODULE  -DKBUILD_BASENAME='"main"' -DKBUILD_MODNAME='"main"' -D__KBUILD_MODNAME=kmod_main -c -o /mnt/driver/main.o /mnt/driver/main.c

source_/mnt/driver/main.o := /mnt/driver/main.c

deps_/mnt/driver/main.o := \
    $(wildcard include/config/cfi/clang.h) \
  /opt/ddk/src/android12-5.10/include/linux/kconfig.h \
    $(wildcard include/config/cc/version/text.h) \
    $(wildcard include/config/cpu/big/endian.h) \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \
  /opt/ddk/src/android12-5.10/include/linux/compiler_types.h \
    $(wildcard include/config/have/arch/compiler/h.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/cc/has/asm/inline.h) \
  /opt/ddk/src/android12-5.10/include/linux/compiler_attributes.h \
  /opt/ddk/src/android12-5.10/include/linux/compiler-clang.h \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/compiler.h \
  /opt/ddk/src/android12-5.10/include/linux/init.h \
    $(wildcard include/config/have/arch/prel32/relocations.h) \
    $(wildcard include/config/strict/kernel/rwx.h) \
    $(wildcard include/config/strict/module/rwx.h) \
    $(wildcard include/config/lto/clang.h) \
  /opt/ddk/src/android12-5.10/include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/stack/validation.h) \
    $(wildcard include/config/64bit.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/rwonce.h \
    $(wildcard include/config/lto.h) \
    $(wildcard include/config/as/has/ldapr.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/alternative-macros.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/cpucaps.h \
  /opt/ddk/src/android12-5.10/include/linux/stringify.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/rwonce.h \
  /opt/ddk/src/android12-5.10/include/linux/kasan-checks.h \
    $(wildcard include/config/kasan/generic.h) \
    $(wildcard include/config/kasan/sw/tags.h) \
  /opt/ddk/src/android12-5.10/include/linux/types.h \
    $(wildcard include/config/have/uid16.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/types.h \
  arch/arm64/include/generated/uapi/asm/types.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/types.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/int-ll64.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/int-ll64.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/bitsperlong.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitsperlong.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/bitsperlong.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/posix_types.h \
  /opt/ddk/src/android12-5.10/include/linux/stddef.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/stddef.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/posix_types.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/posix_types.h \
  /opt/ddk/src/android12-5.10/include/linux/kcsan-checks.h \
    $(wildcard include/config/kcsan.h) \
    $(wildcard include/config/kcsan/ignore/atomics.h) \
  /opt/ddk/src/android12-5.10/include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/preempt/rt.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/panic/timeout.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /opt/ddk/src/android12-5.10/include/linux/limits.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/limits.h \
  /opt/ddk/src/android12-5.10/include/vdso/limits.h \
  /opt/ddk/src/android12-5.10/include/linux/linkage.h \
    $(wildcard include/config/arch/use/sym/annotations.h) \
  /opt/ddk/src/android12-5.10/include/linux/export.h \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/rel/crcs.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/trim/unused/ksyms.h) \
    $(wildcard include/config/unused/symbols.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/linkage.h \
    $(wildcard include/config/arm64/bti/kernel.h) \
  /opt/ddk/src/android12-5.10/include/linux/bitops.h \
  /opt/ddk/src/android12-5.10/include/linux/bits.h \
  /opt/ddk/src/android12-5.10/include/linux/const.h \
  /opt/ddk/src/android12-5.10/include/vdso/const.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/const.h \
  /opt/ddk/src/android12-5.10/include/vdso/bits.h \
  /opt/ddk/src/android12-5.10/include/linux/build_bug.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/bitops.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/builtin-__ffs.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/builtin-ffs.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/builtin-__fls.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/builtin-fls.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/ffz.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/fls64.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/find.h \
    $(wildcard include/config/generic/find/first/bit.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/sched.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/hweight.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/arch_hweight.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/const_hweight.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/atomic.h \
  /opt/ddk/src/android12-5.10/include/linux/atomic.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/atomic.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/barrier.h \
    $(wildcard include/config/arm64/pseudo/nmi.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/barrier.h \
    $(wildcard include/config/smp.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/cmpxchg.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/lse.h \
    $(wildcard include/config/arm64/lse/atomics.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/atomic_ll_sc.h \
    $(wildcard include/config/cc/has/k/constraint.h) \
  /opt/ddk/src/android12-5.10/include/linux/jump_label.h \
    $(wildcard include/config/jump/label.h) \
    $(wildcard include/config/have/arch/jump/label/relative.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/jump_label.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/insn.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/brk-imm.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/alternative.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/atomic_lse.h \
  /opt/ddk/src/android12-5.10/include/linux/atomic-arch-fallback.h \
    $(wildcard include/config/generic/atomic64.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/atomic-instrumented.h \
  /opt/ddk/src/android12-5.10/include/linux/instrumented.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/atomic-long.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/lock.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/non-atomic.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/le.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/byteorder.h \
  /opt/ddk/src/android12-5.10/include/linux/byteorder/little_endian.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/byteorder/little_endian.h \
  /opt/ddk/src/android12-5.10/include/linux/swab.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/swab.h \
  arch/arm64/include/generated/uapi/asm/swab.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/swab.h \
  /opt/ddk/src/android12-5.10/include/linux/byteorder/generic.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/bitops/ext2-atomic-setbit.h \
  /opt/ddk/src/android12-5.10/include/linux/kstrtox.h \
  /opt/ddk/src/android12-5.10/include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  /opt/ddk/src/android12-5.10/include/linux/minmax.h \
  /opt/ddk/src/android12-5.10/include/linux/typecheck.h \
  /opt/ddk/src/android12-5.10/include/linux/printk.h \
    $(wildcard include/config/message/loglevel/default.h) \
    $(wildcard include/config/console/loglevel/default.h) \
    $(wildcard include/config/console/loglevel/quiet.h) \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk/nmi.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/dynamic/debug/core.h) \
  /opt/ddk/src/android12-5.10/include/linux/kern_levels.h \
  /opt/ddk/src/android12-5.10/include/linux/ratelimit_types.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/param.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/param.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/param.h \
  /opt/ddk/src/android12-5.10/include/linux/spinlock_types.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/spinlock_types.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/qspinlock_types.h \
    $(wildcard include/config/nr/cpus.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/qrwlock_types.h \
  /opt/ddk/src/android12-5.10/include/linux/lockdep_types.h \
    $(wildcard include/config/prove/raw/lock/nesting.h) \
    $(wildcard include/config/preempt/lock.h) \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
  /opt/ddk/src/android12-5.10/include/linux/rwlock_types.h \
  arch/arm64/include/generated/asm/div64.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/div64.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/kernel.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/sysinfo.h \
  /opt/ddk/src/android12-5.10/include/linux/kmsg_dump.h \
  /opt/ddk/src/android12-5.10/include/linux/errno.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/errno.h \
  arch/arm64/include/generated/uapi/asm/errno.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/errno.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/errno-base.h \
  /opt/ddk/src/android12-5.10/include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  /opt/ddk/src/android12-5.10/include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  /opt/ddk/src/android12-5.10/include/linux/kprobes.h \
    $(wildcard include/config/kprobes.h) \
    $(wildcard include/config/kretprobes.h) \
    $(wildcard include/config/kprobes/sanity/test.h) \
    $(wildcard include/config/optprobes.h) \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/kprobes/on/ftrace.h) \
  /opt/ddk/src/android12-5.10/include/linux/notifier.h \
    $(wildcard include/config/tree/srcu.h) \
  /opt/ddk/src/android12-5.10/include/linux/mutex.h \
    $(wildcard include/config/mutex/spin/on/owner.h) \
    $(wildcard include/config/debug/mutexes.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/current.h \
  /opt/ddk/src/android12-5.10/include/linux/lockdep.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
    $(wildcard include/config/preempt/count.h) \
  /opt/ddk/src/android12-5.10/include/linux/smp.h \
    $(wildcard include/config/up/late/init.h) \
    $(wildcard include/config/debug/preempt.h) \
  /opt/ddk/src/android12-5.10/include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  /opt/ddk/src/android12-5.10/include/linux/threads.h \
    $(wildcard include/config/base/small.h) \
  /opt/ddk/src/android12-5.10/include/linux/bitmap.h \
  /opt/ddk/src/android12-5.10/include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
    $(wildcard include/config/fortify/source.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/string.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/string.h \
    $(wildcard include/config/arch/has/uaccess/flushcache.h) \
  /opt/ddk/src/android12-5.10/include/linux/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/bug/on/data/corruption.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/bug.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/asm-bug.h \
    $(wildcard include/config/debug/bugverbose.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /opt/ddk/src/android12-5.10/include/linux/instrumentation.h \
    $(wildcard include/config/debug/entry.h) \
  /opt/ddk/src/android12-5.10/include/linux/smp_types.h \
  /opt/ddk/src/android12-5.10/include/linux/llist.h \
    $(wildcard include/config/arch/have/nmi/safe/cmpxchg.h) \
  /opt/ddk/src/android12-5.10/include/linux/preempt.h \
    $(wildcard include/config/trace/preempt/toggle.h) \
    $(wildcard include/config/preemption.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/preempt.h \
  /opt/ddk/src/android12-5.10/include/linux/thread_info.h \
    $(wildcard include/config/thread/info/in/task.h) \
    $(wildcard include/config/have/arch/within/stack/frames.h) \
    $(wildcard include/config/hardened/usercopy.h) \
  /opt/ddk/src/android12-5.10/include/linux/restart_block.h \
  /opt/ddk/src/android12-5.10/include/linux/time64.h \
  /opt/ddk/src/android12-5.10/include/linux/math64.h \
    $(wildcard include/config/arch/supports/int128.h) \
  /opt/ddk/src/android12-5.10/include/vdso/math64.h \
  /opt/ddk/src/android12-5.10/include/vdso/time64.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/time.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/time_types.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/thread_info.h \
    $(wildcard include/config/arm64/sw/ttbr0/pan.h) \
    $(wildcard include/config/shadow/call/stack.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/memory.h \
    $(wildcard include/config/arm64/va/bits.h) \
    $(wildcard include/config/kasan/shadow/offset.h) \
    $(wildcard include/config/kasan.h) \
    $(wildcard include/config/vmap/stack.h) \
    $(wildcard include/config/arm64/4k/pages.h) \
    $(wildcard include/config/kasan/hw/tags.h) \
    $(wildcard include/config/debug/virtual.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/efi.h) \
    $(wildcard include/config/arm/gic/v3/its.h) \
  /opt/ddk/src/android12-5.10/include/linux/sizes.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/page-def.h \
    $(wildcard include/config/arm64/page/shift.h) \
  /opt/ddk/src/android12-5.10/include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/vm/pgflags.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/memory_model.h \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem.h) \
  /opt/ddk/src/android12-5.10/include/linux/pfn.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/stack_pointer.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/smp.h \
    $(wildcard include/config/arm64/acpi/parking/protocol.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/percpu.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/sysreg.h \
    $(wildcard include/config/broken/gas/inst.h) \
    $(wildcard include/config/arm64/pa/bits/52.h) \
    $(wildcard include/config/arm64/16k/pages.h) \
    $(wildcard include/config/arm64/64k/pages.h) \
  /opt/ddk/src/android12-5.10/include/linux/kasan-tags.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  /opt/ddk/src/android12-5.10/include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
    $(wildcard include/config/amd/mem/encrypt.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/processor.h \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/kuser/helpers.h) \
    $(wildcard include/config/arm64/force/52bit.h) \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/arm64/ptr/auth.h) \
    $(wildcard include/config/arm64/mte.h) \
    $(wildcard include/config/arm64/tagged/addr/abi.h) \
    $(wildcard include/config/gcc/plugin/stackleak.h) \
  /opt/ddk/src/android12-5.10/include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/cache.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/cputype.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/mte-def.h \
  /opt/ddk/src/android12-5.10/include/linux/kasan-enabled.h \
  /opt/ddk/src/android12-5.10/include/linux/static_key.h \
  /opt/ddk/src/android12-5.10/include/linux/android_vendor.h \
    $(wildcard include/config/android/vendor/oem/data.h) \
  /opt/ddk/src/android12-5.10/include/vdso/processor.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/vdso/processor.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/cpufeature.h \
    $(wildcard include/config/arm64/sve.h) \
    $(wildcard include/config/arm64/cnp.h) \
    $(wildcard include/config/arm64/debug/priority/masking.h) \
    $(wildcard include/config/arm64/bti.h) \
    $(wildcard include/config/arm64/tlb/range.h) \
    $(wildcard include/config/arm64/pa/bits.h) \
    $(wildcard include/config/arm64/hw/afdbm.h) \
    $(wildcard include/config/arm64/amu/extn.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/hwcap.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/hwcap.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/hw_breakpoint.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/virt.h \
    $(wildcard include/config/kvm.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/ptrace.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/ptrace.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/sve_context.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/sections.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/sections.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/kasan.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/mte-kasan.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/pgtable-types.h \
    $(wildcard include/config/pgtable/levels.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/pgtable-nopud.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/pgtable-nop4d.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/pgtable-hwdef.h \
    $(wildcard include/config/arm64/cont/pte/shift.h) \
    $(wildcard include/config/arm64/cont/pmd/shift.h) \
    $(wildcard include/config/arm64/va/bits/52.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/pointer_auth.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/prctl.h \
  /opt/ddk/src/android12-5.10/include/linux/random.h \
    $(wildcard include/config/arch/random.h) \
  /opt/ddk/src/android12-5.10/include/linux/once.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/random.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/ioctl.h \
  arch/arm64/include/generated/uapi/asm/ioctl.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/ioctl.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/ioctl.h \
  /opt/ddk/src/android12-5.10/include/linux/irqnr.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/irqnr.h \
  /opt/ddk/src/android12-5.10/include/linux/prandom.h \
  /opt/ddk/src/android12-5.10/include/linux/percpu.h \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
  /opt/ddk/src/android12-5.10/include/linux/siphash.h \
    $(wildcard include/config/have/efficient/unaligned/access.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/archrandom.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/spectre.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/fpsimd.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/sigcontext.h \
  /opt/ddk/src/android12-5.10/include/linux/osq_lock.h \
  /opt/ddk/src/android12-5.10/include/linux/debug_locks.h \
  /opt/ddk/src/android12-5.10/include/linux/rwsem.h \
    $(wildcard include/config/rwsem/spin/on/owner.h) \
    $(wildcard include/config/debug/rwsems.h) \
  /opt/ddk/src/android12-5.10/include/linux/spinlock.h \
  /opt/ddk/src/android12-5.10/include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/irqflags.h \
  /opt/ddk/src/android12-5.10/include/linux/bottom_half.h \
  arch/arm64/include/generated/asm/mmiowb.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/mmiowb.h \
    $(wildcard include/config/mmiowb.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/spinlock.h \
  arch/arm64/include/generated/asm/qrwlock.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/qrwlock.h \
  arch/arm64/include/generated/asm/qspinlock.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/qspinlock.h \
  /opt/ddk/src/android12-5.10/include/linux/rwlock.h \
    $(wildcard include/config/preempt.h) \
  /opt/ddk/src/android12-5.10/include/linux/spinlock_api_smp.h \
    $(wildcard include/config/inline/spin/lock.h) \
    $(wildcard include/config/inline/spin/lock/bh.h) \
    $(wildcard include/config/inline/spin/lock/irq.h) \
    $(wildcard include/config/inline/spin/lock/irqsave.h) \
    $(wildcard include/config/inline/spin/trylock.h) \
    $(wildcard include/config/inline/spin/trylock/bh.h) \
    $(wildcard include/config/uninline/spin/unlock.h) \
    $(wildcard include/config/inline/spin/unlock/bh.h) \
    $(wildcard include/config/inline/spin/unlock/irq.h) \
    $(wildcard include/config/inline/spin/unlock/irqrestore.h) \
    $(wildcard include/config/generic/lockbreak.h) \
  /opt/ddk/src/android12-5.10/include/linux/rwlock_api_smp.h \
    $(wildcard include/config/inline/read/lock.h) \
    $(wildcard include/config/inline/write/lock.h) \
    $(wildcard include/config/inline/read/lock/bh.h) \
    $(wildcard include/config/inline/write/lock/bh.h) \
    $(wildcard include/config/inline/read/lock/irq.h) \
    $(wildcard include/config/inline/write/lock/irq.h) \
    $(wildcard include/config/inline/read/lock/irqsave.h) \
    $(wildcard include/config/inline/write/lock/irqsave.h) \
    $(wildcard include/config/inline/read/trylock.h) \
    $(wildcard include/config/inline/write/trylock.h) \
    $(wildcard include/config/inline/read/unlock.h) \
    $(wildcard include/config/inline/write/unlock.h) \
    $(wildcard include/config/inline/read/unlock/bh.h) \
    $(wildcard include/config/inline/write/unlock/bh.h) \
    $(wildcard include/config/inline/read/unlock/irq.h) \
    $(wildcard include/config/inline/write/unlock/irq.h) \
    $(wildcard include/config/inline/read/unlock/irqrestore.h) \
    $(wildcard include/config/inline/write/unlock/irqrestore.h) \
  /opt/ddk/src/android12-5.10/include/linux/err.h \
  /opt/ddk/src/android12-5.10/include/linux/srcu.h \
    $(wildcard include/config/tiny/srcu.h) \
    $(wildcard include/config/srcu.h) \
  /opt/ddk/src/android12-5.10/include/linux/rcupdate.h \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/tasks/rcu/generic.h) \
    $(wildcard include/config/rcu/stall/common.h) \
    $(wildcard include/config/no/hz/full.h) \
    $(wildcard include/config/rcu/nocb/cpu.h) \
    $(wildcard include/config/tasks/rcu.h) \
    $(wildcard include/config/tasks/trace/rcu.h) \
    $(wildcard include/config/tasks/rude/rcu.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/prove/rcu.h) \
    $(wildcard include/config/rcu/boost.h) \
    $(wildcard include/config/arch/weak/release/acquire.h) \
  /opt/ddk/src/android12-5.10/include/linux/rcutree.h \
  /opt/ddk/src/android12-5.10/include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/wq/watchdog.h) \
  /opt/ddk/src/android12-5.10/include/linux/timer.h \
    $(wildcard include/config/debug/objects/timers.h) \
    $(wildcard include/config/no/hz/common.h) \
  /opt/ddk/src/android12-5.10/include/linux/ktime.h \
  /opt/ddk/src/android12-5.10/include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
    $(wildcard include/config/posix/timers.h) \
  /opt/ddk/src/android12-5.10/include/linux/time32.h \
  /opt/ddk/src/android12-5.10/include/linux/timex.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/timex.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/timex.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/arch_timer.h \
    $(wildcard include/config/arm/arch/timer/ool/workaround.h) \
  /opt/ddk/src/android12-5.10/include/clocksource/arm_arch_timer.h \
    $(wildcard include/config/arm/arch/timer.h) \
  /opt/ddk/src/android12-5.10/include/linux/timecounter.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/timex.h \
  /opt/ddk/src/android12-5.10/include/vdso/time32.h \
  /opt/ddk/src/android12-5.10/include/vdso/time.h \
  /opt/ddk/src/android12-5.10/include/linux/jiffies.h \
  /opt/ddk/src/android12-5.10/include/vdso/jiffies.h \
  include/generated/timeconst.h \
  /opt/ddk/src/android12-5.10/include/vdso/ktime.h \
  /opt/ddk/src/android12-5.10/include/linux/timekeeping.h \
  /opt/ddk/src/android12-5.10/include/linux/timekeeping32.h \
  /opt/ddk/src/android12-5.10/include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/debug/objects/free.h) \
  /opt/ddk/src/android12-5.10/include/linux/android_kabi.h \
    $(wildcard include/config/android/kabi/reserve.h) \
  /opt/ddk/src/android12-5.10/include/linux/rcu_segcblist.h \
  /opt/ddk/src/android12-5.10/include/linux/srcutree.h \
  /opt/ddk/src/android12-5.10/include/linux/rcu_node_tree.h \
    $(wildcard include/config/rcu/fanout.h) \
    $(wildcard include/config/rcu/fanout/leaf.h) \
  /opt/ddk/src/android12-5.10/include/linux/completion.h \
  /opt/ddk/src/android12-5.10/include/linux/swait.h \
  /opt/ddk/src/android12-5.10/include/linux/wait.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/wait.h \
  /opt/ddk/src/android12-5.10/include/linux/ftrace.h \
    $(wildcard include/config/function/tracer.h) \
    $(wildcard include/config/dynamic/ftrace.h) \
    $(wildcard include/config/dynamic/ftrace/with/regs.h) \
    $(wildcard include/config/dynamic/ftrace/with/direct/calls.h) \
    $(wildcard include/config/have/dynamic/ftrace/with/direct/calls.h) \
    $(wildcard include/config/stack/tracer.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/frame/pointer.h) \
    $(wildcard include/config/function/profiler.h) \
    $(wildcard include/config/ftrace/syscalls.h) \
  /opt/ddk/src/android12-5.10/include/linux/trace_clock.h \
  arch/arm64/include/generated/asm/trace_clock.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/trace_clock.h \
  /opt/ddk/src/android12-5.10/include/linux/kallsyms.h \
    $(wildcard include/config/kallsyms/all.h) \
    $(wildcard include/config/kallsyms.h) \
  /opt/ddk/src/android12-5.10/include/linux/mm.h \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/mmap/rnd/bits.h) \
    $(wildcard include/config/have/arch/mmap/rnd/compat/bits.h) \
    $(wildcard include/config/mem/soft/dirty.h) \
    $(wildcard include/config/arch/uses/high/vma/flags.h) \
    $(wildcard include/config/arch/has/pkeys.h) \
    $(wildcard include/config/ppc.h) \
    $(wildcard include/config/x86.h) \
    $(wildcard include/config/parisc.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/sparc64.h) \
    $(wildcard include/config/arm64.h) \
    $(wildcard include/config/have/arch/userfaultfd/minor.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/speculative/page/fault.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/transparent/hugepage.h) \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/zone/device.h) \
    $(wildcard include/config/dev/pagemap/ops.h) \
    $(wildcard include/config/device/private.h) \
    $(wildcard include/config/pci/p2pdma.h) \
    $(wildcard include/config/numa/balancing.h) \
    $(wildcard include/config/memcg.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/arch/has/pte/special.h) \
    $(wildcard include/config/arch/has/pte/devmap.h) \
    $(wildcard include/config/debug/vm/rb.h) \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/arch/has/set/direct/map.h) \
    $(wildcard include/config/hibernation.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/hugetlbfs.h) \
    $(wildcard include/config/mapping/dirty/helpers.h) \
  /opt/ddk/src/android12-5.10/include/linux/gfp.h \
    $(wildcard include/config/cma.h) \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/contig/alloc.h) \
  /opt/ddk/src/android12-5.10/include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/isolation.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/page/extension.h) \
    $(wildcard include/config/deferred/struct/page/init.h) \
    $(wildcard include/config/have/memoryless/nodes.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/memory/hotremove.h) \
    $(wildcard include/config/have/arch/pfn/valid.h) \
    $(wildcard include/config/holes/in/zone.h) \
  /opt/ddk/src/android12-5.10/include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
    $(wildcard include/config/numa/keep/meminfo.h) \
  /opt/ddk/src/android12-5.10/include/linux/seqlock.h \
  /opt/ddk/src/android12-5.10/include/linux/ww_mutex.h \
    $(wildcard include/config/debug/ww/mutex/slowpath.h) \
  /opt/ddk/src/android12-5.10/include/linux/nodemask.h \
  /opt/ddk/src/android12-5.10/include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  /opt/ddk/src/android12-5.10/include/linux/page-flags-layout.h \
  include/generated/bounds.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/sparsemem.h \
  /opt/ddk/src/android12-5.10/include/linux/mm_types.h \
    $(wildcard include/config/have/aligned/struct/page.h) \
    $(wildcard include/config/userfaultfd.h) \
    $(wildcard include/config/swap.h) \
    $(wildcard include/config/have/arch/compat/mmap/bases.h) \
    $(wildcard include/config/membarrier.h) \
    $(wildcard include/config/aio.h) \
    $(wildcard include/config/mmu/notifier.h) \
    $(wildcard include/config/arch/want/batched/unmap/tlb/flush.h) \
    $(wildcard include/config/iommu/support.h) \
  /opt/ddk/src/android12-5.10/include/linux/mm_types_task.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/arch/enable/split/pmd/ptlock.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/page.h \
  /opt/ddk/src/android12-5.10/include/linux/personality.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/personality.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/getorder.h \
  /opt/ddk/src/android12-5.10/include/linux/auxvec.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/auxvec.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/auxvec.h \
  /opt/ddk/src/android12-5.10/include/linux/rbtree.h \
  /opt/ddk/src/android12-5.10/include/linux/uprobes.h \
    $(wildcard include/config/uprobes.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/uprobes.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/debug-monitors.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/esr.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/probes.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/mmu.h \
  /opt/ddk/src/android12-5.10/include/linux/refcount.h \
  /opt/ddk/src/android12-5.10/include/linux/page-flags.h \
    $(wildcard include/config/arch/uses/pg/uncached.h) \
    $(wildcard include/config/memory/failure.h) \
    $(wildcard include/config/page/idle/flag.h) \
    $(wildcard include/config/thp/swap.h) \
    $(wildcard include/config/ksm.h) \
  /opt/ddk/src/android12-5.10/include/linux/memory_hotplug.h \
    $(wildcard include/config/arch/has/add/pages.h) \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/have/bootmem/info/node.h) \
  /opt/ddk/src/android12-5.10/include/linux/topology.h \
    $(wildcard include/config/use/percpu/numa/node/id.h) \
    $(wildcard include/config/sched/smt.h) \
  /opt/ddk/src/android12-5.10/include/linux/arch_topology.h \
    $(wildcard include/config/generic/arch/topology.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/topology.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/topology.h \
  /opt/ddk/src/android12-5.10/include/linux/mmap_lock.h \
  /opt/ddk/src/android12-5.10/include/linux/range.h \
  /opt/ddk/src/android12-5.10/include/linux/percpu-refcount.h \
  /opt/ddk/src/android12-5.10/include/linux/bit_spinlock.h \
  /opt/ddk/src/android12-5.10/include/linux/shrinker.h \
  /opt/ddk/src/android12-5.10/include/linux/resource.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/resource.h \
  arch/arm64/include/generated/uapi/asm/resource.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/resource.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/resource.h \
  /opt/ddk/src/android12-5.10/include/linux/page_ext.h \
    $(wildcard include/config/page/pinner.h) \
  /opt/ddk/src/android12-5.10/include/linux/stacktrace.h \
    $(wildcard include/config/stacktrace.h) \
    $(wildcard include/config/arch/stackwalk.h) \
    $(wildcard include/config/have/reliable/stacktrace.h) \
  /opt/ddk/src/android12-5.10/include/linux/stackdepot.h \
    $(wildcard include/config/stackdepot.h) \
  /opt/ddk/src/android12-5.10/include/linux/page_ref.h \
    $(wildcard include/config/debug/page/ref.h) \
  /opt/ddk/src/android12-5.10/include/linux/tracepoint-defs.h \
    $(wildcard include/config/tracepoints.h) \
  /opt/ddk/src/android12-5.10/include/linux/memremap.h \
  /opt/ddk/src/android12-5.10/include/linux/ioport.h \
    $(wildcard include/config/io/strict/devmem.h) \
  /opt/ddk/src/android12-5.10/include/linux/overflow.h \
  /opt/ddk/src/android12-5.10/include/linux/sched.h \
    $(wildcard include/config/virt/cpu/accounting/native.h) \
    $(wildcard include/config/sched/info.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/fair/group/sched.h) \
    $(wildcard include/config/rt/group/sched.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/uclamp/task.h) \
    $(wildcard include/config/uclamp/buckets/count.h) \
    $(wildcard include/config/cgroup/sched.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/psi.h) \
    $(wildcard include/config/compat/brk.h) \
    $(wildcard include/config/cgroups.h) \
    $(wildcard include/config/blk/cgroup.h) \
    $(wildcard include/config/stackprotector.h) \
    $(wildcard include/config/arch/has/scaled/cputime.h) \
    $(wildcard include/config/cpu/freq/times.h) \
    $(wildcard include/config/virt/cpu/accounting/gen.h) \
    $(wildcard include/config/posix/cputimers.h) \
    $(wildcard include/config/posix/cpu/timers/task/work.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/detect/hung/task.h) \
    $(wildcard include/config/io/uring.h) \
    $(wildcard include/config/audit.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/ubsan.h) \
    $(wildcard include/config/ubsan/trap.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/x86/cpu/resctrl.h) \
    $(wildcard include/config/futex.h) \
    $(wildcard include/config/perf/events.h) \
    $(wildcard include/config/rseq.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/fault/injection.h) \
    $(wildcard include/config/latencytop.h) \
    $(wildcard include/config/kunit.h) \
    $(wildcard include/config/kcov.h) \
    $(wildcard include/config/bcache.h) \
    $(wildcard include/config/livepatch.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/x86/mce.h) \
    $(wildcard include/config/rt/softint/optimization.h) \
    $(wildcard include/config/arch/task/struct/on/stack.h) \
    $(wildcard include/config/debug/rseq.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/sched.h \
  /opt/ddk/src/android12-5.10/include/linux/pid.h \
  /opt/ddk/src/android12-5.10/include/linux/rculist.h \
    $(wildcard include/config/prove/rcu/list.h) \
  /opt/ddk/src/android12-5.10/include/linux/sem.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/sem.h \
  /opt/ddk/src/android12-5.10/include/linux/ipc.h \
  /opt/ddk/src/android12-5.10/include/linux/uidgid.h \
    $(wildcard include/config/multiuser.h) \
    $(wildcard include/config/user/ns.h) \
  /opt/ddk/src/android12-5.10/include/linux/highuid.h \
  /opt/ddk/src/android12-5.10/include/linux/rhashtable-types.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/ipc.h \
  arch/arm64/include/generated/uapi/asm/ipcbuf.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/ipcbuf.h \
  arch/arm64/include/generated/uapi/asm/sembuf.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/sembuf.h \
  /opt/ddk/src/android12-5.10/include/linux/shm.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/shm.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/hugetlb_encode.h \
  arch/arm64/include/generated/uapi/asm/shmbuf.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/shmbuf.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/shmparam.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/shmparam.h \
  /opt/ddk/src/android12-5.10/include/linux/plist.h \
    $(wildcard include/config/debug/plist.h) \
  /opt/ddk/src/android12-5.10/include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
    $(wildcard include/config/time/low/res.h) \
    $(wildcard include/config/timerfd.h) \
  /opt/ddk/src/android12-5.10/include/linux/hrtimer_defs.h \
  /opt/ddk/src/android12-5.10/include/linux/timerqueue.h \
  /opt/ddk/src/android12-5.10/include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
    $(wildcard include/config/have/arch/seccomp/filter.h) \
    $(wildcard include/config/seccomp/filter.h) \
    $(wildcard include/config/checkpoint/restore.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/seccomp.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/seccomp.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/unistd.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/unistd.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/unistd.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/seccomp.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/unistd.h \
  /opt/ddk/src/android12-5.10/include/linux/latencytop.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/prio.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/types.h \
  /opt/ddk/src/android12-5.10/include/linux/signal_types.h \
    $(wildcard include/config/old/sigaction.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/signal.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/signal.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/signal.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/signal.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/signal.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/signal-defs.h \
  arch/arm64/include/generated/uapi/asm/siginfo.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/siginfo.h \
  /opt/ddk/src/android12-5.10/include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  /opt/ddk/src/android12-5.10/include/linux/posix-timers.h \
  /opt/ddk/src/android12-5.10/include/linux/alarmtimer.h \
    $(wildcard include/config/rtc/class.h) \
  /opt/ddk/src/android12-5.10/include/linux/task_work.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/rseq.h \
  /opt/ddk/src/android12-5.10/include/linux/kcsan.h \
  /opt/ddk/src/android12-5.10/include/linux/pgtable.h \
    $(wildcard include/config/highpte.h) \
    $(wildcard include/config/have/arch/transparent/hugepage/pud.h) \
    $(wildcard include/config/have/arch/soft/dirty.h) \
    $(wildcard include/config/arch/enable/thp/migration.h) \
    $(wildcard include/config/have/arch/huge/vmap.h) \
    $(wildcard include/config/x86/espfix64.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/pgtable.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/proc-fns.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/mte.h \
  /opt/ddk/src/android12-5.10/include/linux/bitfield.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/pgtable-prot.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/tlbflush.h \
    $(wildcard include/config/arm64/workaround/repeat/tlbi.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/fixmap.h \
    $(wildcard include/config/acpi/apei/ghes.h) \
    $(wildcard include/config/arm/sde/interface.h) \
    $(wildcard include/config/unmap/kernel/at/el0.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/boot.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/fixmap.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/pgtable_uffd.h \
    $(wildcard include/config/have/arch/userfaultfd/wp.h) \
  /opt/ddk/src/android12-5.10/include/linux/kasan.h \
    $(wildcard include/config/kasan/stack.h) \
    $(wildcard include/config/kasan/vmalloc.h) \
    $(wildcard include/config/kasan/inline.h) \
  /opt/ddk/src/android12-5.10/include/linux/page_pinner.h \
  /opt/ddk/src/android12-5.10/include/linux/huge_mm.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/coredump.h \
    $(wildcard include/config/core/dump/default/elf/headers.h) \
  /opt/ddk/src/android12-5.10/include/linux/fs.h \
    $(wildcard include/config/read/only/thp/for/fs.h) \
    $(wildcard include/config/fs/posix/acl.h) \
    $(wildcard include/config/cgroup/writeback.h) \
    $(wildcard include/config/ima.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/fsnotify.h) \
    $(wildcard include/config/fs/encryption.h) \
    $(wildcard include/config/fs/verity.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/unicode.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/fs/dax.h) \
    $(wildcard include/config/mandatory/file/locking.h) \
    $(wildcard include/config/migration.h) \
  /opt/ddk/src/android12-5.10/include/linux/wait_bit.h \
  /opt/ddk/src/android12-5.10/include/linux/kdev_t.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/kdev_t.h \
  /opt/ddk/src/android12-5.10/include/linux/dcache.h \
  /opt/ddk/src/android12-5.10/include/linux/rculist_bl.h \
  /opt/ddk/src/android12-5.10/include/linux/list_bl.h \
  /opt/ddk/src/android12-5.10/include/linux/lockref.h \
    $(wildcard include/config/arch/use/cmpxchg/lockref.h) \
  /opt/ddk/src/android12-5.10/include/linux/stringhash.h \
    $(wildcard include/config/dcache/word/access.h) \
  /opt/ddk/src/android12-5.10/include/linux/hash.h \
    $(wildcard include/config/have/arch/hash.h) \
  /opt/ddk/src/android12-5.10/include/linux/path.h \
  /opt/ddk/src/android12-5.10/include/linux/stat.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/stat.h \
  arch/arm64/include/generated/uapi/asm/stat.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/stat.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/compat.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/compat.h \
    $(wildcard include/config/compat/for/u64/alignment.h) \
  /opt/ddk/src/android12-5.10/include/linux/sched/task_stack.h \
    $(wildcard include/config/debug/stack/usage.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/magic.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/stat.h \
  /opt/ddk/src/android12-5.10/include/linux/list_lru.h \
    $(wildcard include/config/memcg/kmem.h) \
  /opt/ddk/src/android12-5.10/include/linux/radix-tree.h \
  /opt/ddk/src/android12-5.10/include/linux/xarray.h \
    $(wildcard include/config/xarray/multi.h) \
  /opt/ddk/src/android12-5.10/include/linux/sched/mm.h \
    $(wildcard include/config/arch/has/membarrier/callbacks.h) \
  /opt/ddk/src/android12-5.10/include/linux/sync_core.h \
    $(wildcard include/config/arch/has/sync/core/before/usermode.h) \
  /opt/ddk/src/android12-5.10/include/linux/local_lock.h \
  /opt/ddk/src/android12-5.10/include/linux/local_lock_internal.h \
  /opt/ddk/src/android12-5.10/include/linux/capability.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/capability.h \
  /opt/ddk/src/android12-5.10/include/linux/semaphore.h \
  /opt/ddk/src/android12-5.10/include/linux/fcntl.h \
    $(wildcard include/config/arch/32bit/off/t.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/fcntl.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/uapi/asm/fcntl.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/fcntl.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/openat2.h \
  /opt/ddk/src/android12-5.10/include/linux/migrate_mode.h \
  /opt/ddk/src/android12-5.10/include/linux/percpu-rwsem.h \
  /opt/ddk/src/android12-5.10/include/linux/rcuwait.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/signal.h \
    $(wildcard include/config/sched/autogroup.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
  /opt/ddk/src/android12-5.10/include/linux/signal.h \
    $(wildcard include/config/proc/fs.h) \
  /opt/ddk/src/android12-5.10/include/linux/sched/jobctl.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/task.h \
    $(wildcard include/config/have/exit/thread.h) \
    $(wildcard include/config/arch/wants/dynamic/task/struct.h) \
    $(wildcard include/config/have/arch/thread/struct/whitelist.h) \
  /opt/ddk/src/android12-5.10/include/linux/uaccess.h \
    $(wildcard include/config/set/fs.h) \
  /opt/ddk/src/android12-5.10/include/linux/fault-inject-usercopy.h \
    $(wildcard include/config/fault/injection/usercopy.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/uaccess.h \
    $(wildcard include/config/arm64/uao.h) \
    $(wildcard include/config/arm64/pan.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/kernel-pgtable.h \
    $(wildcard include/config/randomize/base.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/extable.h \
    $(wildcard include/config/bpf/jit.h) \
  /opt/ddk/src/android12-5.10/include/linux/cred.h \
    $(wildcard include/config/debug/credentials.h) \
  /opt/ddk/src/android12-5.10/include/linux/key.h \
    $(wildcard include/config/key/notifications.h) \
    $(wildcard include/config/net.h) \
  /opt/ddk/src/android12-5.10/include/linux/sysctl.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/sysctl.h \
  /opt/ddk/src/android12-5.10/include/linux/assoc_array.h \
    $(wildcard include/config/associative/array.h) \
  /opt/ddk/src/android12-5.10/include/linux/sched/user.h \
    $(wildcard include/config/fanotify.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/bpf/syscall.h) \
    $(wildcard include/config/watch/queue.h) \
  /opt/ddk/src/android12-5.10/include/linux/ratelimit.h \
  /opt/ddk/src/android12-5.10/include/linux/rcu_sync.h \
  /opt/ddk/src/android12-5.10/include/linux/delayed_call.h \
  /opt/ddk/src/android12-5.10/include/linux/uuid.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/uuid.h \
  /opt/ddk/src/android12-5.10/include/linux/errseq.h \
  /opt/ddk/src/android12-5.10/include/linux/ioprio.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/rt.h \
  /opt/ddk/src/android12-5.10/include/linux/iocontext.h \
  /opt/ddk/src/android12-5.10/include/linux/fs_types.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/fs.h \
  /opt/ddk/src/android12-5.10/include/linux/quota.h \
    $(wildcard include/config/quota/netlink/interface.h) \
  /opt/ddk/src/android12-5.10/include/linux/percpu_counter.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/dqblk_xfs.h \
  /opt/ddk/src/android12-5.10/include/linux/dqblk_v1.h \
  /opt/ddk/src/android12-5.10/include/linux/dqblk_v2.h \
  /opt/ddk/src/android12-5.10/include/linux/dqblk_qtree.h \
  /opt/ddk/src/android12-5.10/include/linux/projid.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/quota.h \
  /opt/ddk/src/android12-5.10/include/linux/nfs_fs_i.h \
  /opt/ddk/src/android12-5.10/include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
    $(wildcard include/config/debug/tlbflush.h) \
    $(wildcard include/config/debug/vm/vmacache.h) \
  /opt/ddk/src/android12-5.10/include/linux/vm_event_item.h \
    $(wildcard include/config/memory/balloon.h) \
    $(wildcard include/config/balloon/compaction.h) \
  /opt/ddk/src/android12-5.10/include/linux/module.h \
    $(wildcard include/config/modules/tree/lookup.h) \
    $(wildcard include/config/bpf/events.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/have/static/call/inline.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/mitigation/its.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/function/error/injection.h) \
    $(wildcard include/config/retpoline.h) \
    $(wildcard include/config/module/sig.h) \
  /opt/ddk/src/android12-5.10/include/linux/kmod.h \
  /opt/ddk/src/android12-5.10/include/linux/umh.h \
  /opt/ddk/src/android12-5.10/include/linux/elf.h \
    $(wildcard include/config/arch/use/gnu/property.h) \
    $(wildcard include/config/arch/have/elf/prot.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/elf.h \
    $(wildcard include/config/compat/vdso.h) \
  arch/arm64/include/generated/asm/user.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/user.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/elf.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/elf-em.h \
  /opt/ddk/src/android12-5.10/include/linux/kobject.h \
    $(wildcard include/config/uevent/helper.h) \
    $(wildcard include/config/debug/kobject/release.h) \
  /opt/ddk/src/android12-5.10/include/linux/sysfs.h \
  /opt/ddk/src/android12-5.10/include/linux/kernfs.h \
    $(wildcard include/config/kernfs.h) \
  /opt/ddk/src/android12-5.10/include/linux/idr.h \
  /opt/ddk/src/android12-5.10/include/linux/kobject_ns.h \
  /opt/ddk/src/android12-5.10/include/linux/kref.h \
  /opt/ddk/src/android12-5.10/include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ppc64.h) \
  /opt/ddk/src/android12-5.10/include/linux/rbtree_latch.h \
  /opt/ddk/src/android12-5.10/include/linux/error-injection.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/error-injection.h \
  /opt/ddk/src/android12-5.10/include/linux/static_call_types.h \
    $(wildcard include/config/have/static/call.h) \
  /opt/ddk/src/android12-5.10/include/linux/cfi.h \
    $(wildcard include/config/cfi/clang/shadow.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/module.h \
    $(wildcard include/config/arm64/module/plts.h) \
    $(wildcard include/config/arm64/erratum/843419.h) \
  /opt/ddk/src/android12-5.10/include/asm-generic/module.h \
    $(wildcard include/config/have/mod/arch/specific.h) \
    $(wildcard include/config/modules/use/elf/rel.h) \
    $(wildcard include/config/modules/use/elf/rela.h) \
  /opt/ddk/src/android12-5.10/include/linux/ptrace.h \
  /opt/ddk/src/android12-5.10/include/linux/pid_namespace.h \
    $(wildcard include/config/pid/ns.h) \
  /opt/ddk/src/android12-5.10/include/linux/nsproxy.h \
  /opt/ddk/src/android12-5.10/include/linux/ns_common.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/ptrace.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/ftrace.h \
  /opt/ddk/src/android12-5.10/include/linux/compat.h \
    $(wildcard include/config/arch/has/syscall/wrapper.h) \
    $(wildcard include/config/x86/x32/abi.h) \
    $(wildcard include/config/compat/old/sigaction.h) \
    $(wildcard include/config/odd/rt/sigaction.h) \
  /opt/ddk/src/android12-5.10/include/linux/socket.h \
  arch/arm64/include/generated/uapi/asm/socket.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/socket.h \
  arch/arm64/include/generated/uapi/asm/sockios.h \
  /opt/ddk/src/android12-5.10/include/uapi/asm-generic/sockios.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/sockios.h \
  /opt/ddk/src/android12-5.10/include/linux/uio.h \
    $(wildcard include/config/arch/has/copy/mc.h) \
  /opt/ddk/src/android12-5.10/include/uapi/linux/uio.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/socket.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/if.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/libc-compat.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/hdlc/ioctl.h \
  /opt/ddk/src/android12-5.10/include/uapi/linux/aio_abi.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/syscall_wrapper.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/kprobes.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/kprobes.h \
  /opt/ddk/src/android12-5.10/include/linux/slab.h \
    $(wildcard include/config/debug/slab.h) \
    $(wildcard include/config/failslab.h) \
    $(wildcard include/config/have/hardened/usercopy/allocator.h) \
    $(wildcard include/config/slab.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/slob.h) \
  include/generated/uapi/linux/version.h \
  /mnt/driver/kp_hook.h \
  /mnt/driver/kp_hook.c \
  /opt/ddk/src/android12-5.10/include/linux/stop_machine.h \
  /opt/ddk/src/android12-5.10/include/linux/cpu.h \
    $(wildcard include/config/pm/sleep/smp.h) \
    $(wildcard include/config/pm/sleep/smp/nonzero/cpu.h) \
    $(wildcard include/config/arch/has/cpu/finalize/init.h) \
    $(wildcard include/config/hotplug/smt.h) \
  /opt/ddk/src/android12-5.10/include/linux/node.h \
    $(wildcard include/config/hmem/reporting.h) \
    $(wildcard include/config/memory/hotplug/sparse.h) \
  /opt/ddk/src/android12-5.10/include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
    $(wildcard include/config/energy/model.h) \
    $(wildcard include/config/generic/msi/irq/domain.h) \
    $(wildcard include/config/pinctrl.h) \
    $(wildcard include/config/generic/msi/irq.h) \
    $(wildcard include/config/dma/ops.h) \
    $(wildcard include/config/dma/declare/coherent.h) \
    $(wildcard include/config/dma/cma.h) \
    $(wildcard include/config/arch/has/sync/dma/for/device.h) \
    $(wildcard include/config/arch/has/sync/dma/for/cpu.h) \
    $(wildcard include/config/arch/has/sync/dma/for/cpu/all.h) \
    $(wildcard include/config/dma/ops/bypass.h) \
    $(wildcard include/config/of.h) \
    $(wildcard include/config/devtmpfs.h) \
    $(wildcard include/config/sysfs/deprecated.h) \
  /opt/ddk/src/android12-5.10/include/linux/dev_printk.h \
  /opt/ddk/src/android12-5.10/include/linux/energy_model.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/cpufreq.h \
    $(wildcard include/config/cpu/freq.h) \
  /opt/ddk/src/android12-5.10/include/linux/sched/topology.h \
    $(wildcard include/config/sched/debug.h) \
    $(wildcard include/config/sched/mc.h) \
  /opt/ddk/src/android12-5.10/include/linux/sched/idle.h \
  /opt/ddk/src/android12-5.10/include/linux/sched/sd_flags.h \
  /opt/ddk/src/android12-5.10/include/linux/klist.h \
  /opt/ddk/src/android12-5.10/include/linux/pm.h \
    $(wildcard include/config/vt/console/sleep.h) \
    $(wildcard include/config/pm.h) \
    $(wildcard include/config/pm/clk.h) \
    $(wildcard include/config/pm/generic/domains.h) \
  /opt/ddk/src/android12-5.10/include/linux/device/bus.h \
    $(wildcard include/config/acpi.h) \
  /opt/ddk/src/android12-5.10/include/linux/device/class.h \
  /opt/ddk/src/android12-5.10/include/linux/device/driver.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/device.h \
  /opt/ddk/src/android12-5.10/include/linux/pm_wakeup.h \
  /opt/ddk/src/android12-5.10/include/linux/cpuhotplug.h \
  /opt/ddk/src/android12-5.10/include/linux/vmalloc.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/vmalloc.h \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/cacheflush.h \
  /opt/ddk/src/android12-5.10/include/linux/kgdb.h \
    $(wildcard include/config/have/arch/kgdb.h) \
    $(wildcard include/config/kgdb.h) \
    $(wildcard include/config/serial/kgdb/nmi.h) \
    $(wildcard include/config/kgdb/honour/blocklist.h) \
  /opt/ddk/src/android12-5.10/arch/arm64/include/asm/kgdb.h \
  /opt/ddk/src/android12-5.10/include/asm-generic/cacheflush.h \

/mnt/driver/main.o: $(deps_/mnt/driver/main.o)

$(deps_/mnt/driver/main.o):
