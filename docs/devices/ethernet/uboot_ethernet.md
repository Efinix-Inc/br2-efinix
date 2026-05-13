# Efinix Ethernet Driver (U-Boot)

This document describes the U-Boot Efinix Triple-Speed Ethernet (TSE) MAC driver and how to use Ethernet-based boot methods (TFTP + ramdisk or NFS) on the Efinix RISC-V Sapphire SoC.

---

## Overview

The driver (`drivers/net/efinix_tse.c`) supports the Efinix TSE MAC together with its integrated MDIO controller and a descriptor-based DMA engine. It is enabled by the `CONFIG_EFINIX_TSE` Kconfig option and binds to the `efinix,tsemac-0.01.a` compatible string in the device tree.

Key features:

- Descriptor-based scatter-gather DMA (DMASG) for both TX and RX
- Integrated MDIO controller with configurable clock divider
- Auto-negotiation via `PHYLIB` (supports MII / GMII / RGMII / SGMII)
- RTL8211F PHY-specific fixups (1000BASE-T master mode, page tuning)
- MAC address programming from the U-Boot environment
- Promiscuous mode during boot for reliable TFTP transfers

---

## Device Tree Binding

Two nodes are required: the DMASG controller (`dma0`) and the TSE MAC (`tsemac0`). They reference each other via `axistream-connected` phandles.

```dts
&dma0 {
	#address-cells = <1>;
	#size-cells = <0>;
	compatible = "spinal,lib-dmasg";
	#dma-cells = <1>;
	axistream-connected = <&tsemac0>;
	axistream-control-connected = <&tsemac0>;
	status = "okay";

	dma-channel@0 {
		status = "okay";
		reg = <0>;
	};

	dma-channel@1 {
		status = "okay";
		reg = <1>;
	};
};

&tsemac0 {
	#address-cells = <1>;
	#size-cells = <0>;
	device_type = "network";
	compatible = "efinix,tsemac-0.01.a";
	axistream-connected = <&dma0>;
	axistream-control-connected = <&dma0>;
	local-mac-address = [ 00 11 22 33 44 55 ];
	efx,rxcsum = <0x0>;
	efx,rxmem = <0x1000>;
	efx,stats = <0x0>;
	efx,txcsum = <0x0>;
	efx,txmem = <0x1000>;
	phy-handle = <&phy0>;
	phy-mode = "rgmii";
	status = "okay";

	mdio {
		#address-cells = <1>;
		#size-cells = <0>;
		status = "okay";
		reg = <0>;

		phy0: ethernet-phy@0 {
			status = "okay";
			reg = <0>;
			compatible = "ethernet-phy-id001c.c916";
			reset-assert-us = <500000>;
			reset-deassert-us = <500000>;
		};
	};
};
```

### Key properties

| Property | Node | Description |
|---|---|---|
| `compatible` | `tsemac0` | Must be `"efinix,tsemac-0.01.a"` |
| `axistream-connected` | both | Cross-references the DMA ↔ MAC pairing |
| `axistream-control-connected` | both | Control path cross-reference |
| `phy-mode` | `tsemac0` | PHY interface type (e.g. `rgmii`, `mii`, `gmii`, `sgmii`) |
| `phy-handle` | `tsemac0` | Phandle to the PHY sub-node inside `mdio` |
| `local-mac-address` | `tsemac0` | Default MAC address; overridden by `ethaddr` U-Boot env var |
| `efx,rxmem` / `efx,txmem` | `tsemac0` | RX/TX buffer size in bytes |
| `efx,rxcsum` / `efx,txcsum` | `tsemac0` | Hardware checksum offload (0 = disabled) |
| `compatible` | `phy0` | PHY device ID; `001c.c916` corresponds to the RTL8211F |
| `reset-assert-us` / `reset-deassert-us` | `phy0` | PHY reset timing in microseconds |

---

## Kconfig

Enable the driver in U-Boot configuration:

```
CONFIG_EFINIX_TSE=y
CONFIG_PHYLIB=y
CONFIG_DM_ETH=y
```

---

## Booting over Ethernet

### Host setup

Install and configure a TFTP server (and optionally NFS):

```bash
sudo apt install tftpd-hpa nfs-kernel-server

# TFTP server root
sudo mkdir -p /srv/tftp
sudo chmod -R 777 /srv/tftp

# Edit /etc/default/tftpd-hpa and set TFTP_DIRECTORY="/srv/tftp"
sudo nano /etc/default/tftpd-hpa
sudo systemctl restart tftpd-hpa

# Copy kernel, DTB, and rootfs into the TFTP root
# cp uImage linux.dtb rootfs.cpio.uboot /srv/tftp/
```

For NFS boot, also configure the NFS export:

```bash
sudo mkdir -p /srv/nfs/rootfs

# Extract the NFS rootfs (adjust the source path to your build output)
sudo cp -a <build_dir>/images/nfs_root/* /srv/nfs/rootfs/

# Add to /etc/exports:
#   /srv/nfs/rootfs  192.168.31.0/24(rw,sync,no_subtree_check,no_root_squash)
sudo nano /etc/exports
sudo exportfs -rav
sudo systemctl restart nfs-kernel-server
```

---

### Boot from ramdisk (TFTP + initramfs)

Fetches kernel, DTB, and a `uboot`-wrapped ramdisk from the TFTP server, then boots entirely in RAM.

> **Note:** The IP addresses below (`192.168.31.55`, `192.168.31.222`) are examples only. Replace `ipaddr` with the board's IP address and `serverip` with your host machine's IP address on the same network.

```
# Set network addresses (replace with your own IPs)
setenv ipaddr   192.168.31.55    # board IP
setenv serverip 192.168.31.222   # host/TFTP server IP

# Download images
tftp 0x3800000  uImage
tftp 0x5000000  linux.dtb
tftp 0x10000000 rootfs.cpio.uboot

# Set kernel command line
setenv bootargs 'rootwait console=ttySL0 earlycon root=/dev/ram init=/sbin/init mmc_core.use_spi_crc=0'

# Boot: kernel @ 0x3800000, ramdisk @ 0x10000000, DTB @ 0x5000000
bootm 0x3800000 0x10000000 0x5000000
```

| Address | Image |
|---|---|
| `0x3800000` | `uImage` (Linux kernel) |
| `0x5000000` | `linux.dtb` (device tree blob) |
| `0x10000000` | `rootfs.cpio.uboot` (ramdisk) |

---

### Boot from NFS

Fetches the kernel and DTB over TFTP, then mounts the root filesystem from an NFS server. No ramdisk is needed.

> **Note:** The IP addresses below (`192.168.31.55`, `192.168.31.111`, `192.168.31.1`) are examples only. Replace them with the board's IP, your NFS/TFTP server IP, and your gateway IP respectively. Update the same IPs inside the `nfsroot=` and `ip=` fields in `bootargs`.

```
# Set network addresses (replace with your own IPs)
setenv serverip   192.168.31.111   # host/TFTP+NFS server IP
setenv ipaddr     192.168.31.55    # board IP
setenv gatewayip  192.168.31.1     # network gateway

# Download kernel and DTB
tftp 0x3800000 uImage
tftp 0x5000000 linux.dtb

# Set kernel command line (update IPs and path to match your server)
setenv bootargs 'console=ttySL0 earlycon root=/dev/nfs rw nfsroot=192.168.31.111:/srv/nfs/rootfs,tcp,v3,nolock ip=192.168.31.55:192.168.31.111:192.168.31.1:255.255.255.0::eth0:off init=/sbin/init'

# Boot: kernel @ 0x3800000, no ramdisk (-), DTB @ 0x5000000
bootm 0x3800000 - 0x5000000
```

The `ip=` parameter follows the format:

```
ip=<client>:<server>:<gateway>:<netmask>::<iface>:off
```

---

## Persisting the boot configuration

To avoid re-entering the commands on every reset, save them as U-Boot environment variables. **Remember to substitute the example IP addresses with your actual board and server IPs before saving.**

```
# Ramdisk boot
setenv bootcmd_ram 'setenv ipaddr 192.168.31.55; setenv serverip 192.168.31.222; tftp 0x3800000 uImage; tftp 0x5000000 linux.dtb; tftp 0x10000000 rootfs.cpio.uboot; setenv bootargs "rootwait console=ttySL0 earlycon root=/dev/ram init=/sbin/init mmc_core.use_spi_crc=0"; bootm 0x3800000 0x10000000 0x5000000'

# NFS boot
setenv bootcmd_nfs 'setenv serverip 192.168.31.111; setenv ipaddr 192.168.31.55; setenv gatewayip 192.168.31.1; tftp 0x3800000 uImage; tftp 0x5000000 linux.dtb; setenv bootargs "console=ttySL0 earlycon root=/dev/nfs rw nfsroot=192.168.31.111:/srv/nfs/rootfs,tcp,v3,nolock ip=192.168.31.55:192.168.31.111:192.168.31.1:255.255.255.0::eth0:off init=/sbin/init"; bootm 0x3800000 - 0x5000000'

saveenv
```

Then run with `run bootcmd_ram` or `run bootcmd_nfs`.

---

## Programming eMMC over Ethernet (production flow)

Once the board has booted into a ramdisk or NFS root, you can use the [`emmc_programmer`](../devices/emmc/emmc.md) flow to flash the eMMC directly over the network. This has two key advantages for production:

- **No SD card required** — the board does not need an SD card slot at all. This simplifies board design and eliminates the need to prepare and insert a programming SD card for each unit.
- **Simultaneous multi-board programming** — multiple boards can be booted and programmed in parallel from the same TFTP/NFS server, significantly speeding up production throughput.

### Prerequisites

- `uImage` — Linux kernel
- `linux.dtb` — Linux device tree
- `rootfs.tar` — root filesystem archive

These are produced by the normal Buildroot build. See the unified hardware design [README](../../boards/efinix/ti375c529/hardware/unified_hw/README.md) for build instructions.

### Steps

1. Boot the board over Ethernet using the ramdisk or NFS commands above.

2. After logging into Linux, set up the Ethernet connection if not already configured (see the [configure_the_board](../setup/configure_the_board.md) guide).

3. Copy the programming binaries from the build host to the board:

```bash
mkdir /root/emmc
cd /root/emmc
scp <user>@<host>:/path/to/images/uImage .
scp <user>@<host>:/path/to/images/linux.dtb .
scp <user>@<host>:/path/to/images/rootfs.tar .
```

Replace `<user>@<host>` and the image path with your actual build host username, hostname, and build output directory (e.g. `build_ti375c529/build/images`).

4. Run `emmc_programmer` to flash the eMMC:

```bash
emmc_programmer /dev/mmcblk1 --no-download
```

5. Reboot once programming completes:

```bash
reboot
```

After reboot, the board boots from eMMC.

> For the full `emmc_programmer` reference, device tree configuration, and Kconfig options, see [`docs/devices/emmc/emmc.md`](../devices/emmc/emmc.md).
