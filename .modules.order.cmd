cmd_/mnt/driver/modules.order := {   echo /mnt/driver/main.ko; :; } | awk '!x[$$0]++' - > /mnt/driver/modules.order
