#!/bin/bash
LINUX_ADDRESS=0x00400000

# Set default objcopy binary
objcopy_bin="riscv32-buildroot-linux-gnu-objcopy"
vmlinux_file="$BINARIES_DIR/vmlinux"
machine_arch=$(file "$vmlinux_file" | grep -o '[0-9]\+-bit')

case "$machine_arch" in
	"64-bit")
		objcopy_bin="riscv64-buildroot-linux-gnu-objcopy"
		;;
	"32-bit")
		objcopy_bin="riscv32-buildroot-linux-gnu-objcopy"
		;;
	*)
		echo "Unsupported architecture: $machine_arch"
		echo "Use default objcopy: $objcopy_bin"
		;;
esac

$objcopy_bin -O binary "$vmlinux_file" "$BINARIES_DIR/Image"
$HOST_DIR/bin/mkimage -A riscv -O linux -T kernel -C none -a $LINUX_ADDRESS -e $LINUX_ADDRESS -n Linux -d $BINARIES_DIR/Image $BINARIES_DIR/uImage
