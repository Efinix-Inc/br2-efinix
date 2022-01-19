#!/bin/bash
LINUX_ADDRESS=0x00400000
$HOST_DIR/bin/riscv32-buildroot-linux-gnu-objcopy  -O binary $BINARIES_DIR/vmlinux $BINARIES_DIR/Image
$HOST_DIR/bin/mkimage -A riscv -O linux -T kernel -C none -a $LINUX_ADDRESS -e $LINUX_ADDRESS -n Linux -d $BINARIES_DIR/Image $BINARIES_DIR/uImage
dtc -O dtb -o $BINARIES_DIR/linux.dtb $(dirname $0)/../t120f324/linux/t120f324.dts
