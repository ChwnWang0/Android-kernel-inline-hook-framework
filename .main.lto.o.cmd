cmd_/mnt/driver/main.lto.o := ld.lld -EL  -maarch64elf -z norelro -mllvm -import-instr-limit=5 -z noexecstack   -r -o /mnt/driver/main.lto.o  --whole-archive /mnt/driver/main.o
