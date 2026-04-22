cmd_/mnt/driver/Module.symvers := sed 's/\.ko$$/.lto\.o/' /mnt/driver/modules.order | scripts/mod/modpost -m  -E  -o /mnt/driver/Module.symvers -e -i Module.symvers   -n -T -
