# Write to MTD device

## Introduction

An **MTD device** (Memory Technology Device) in Linux refers to a special type of device file used to interface with **raw flash memory chips**, such as **NAND** or **NOR flash**. These devices are common in embedded systems like routers, IoT devices, smartphones, and industrial controllers.

An MTD device provides a **raw interface** to flash memory, unlike block devices (like `/dev/sda`) which abstract away the underlying hardware. MTD devices are exposed in Linux as:

- `/dev/mtdX` – character device (read/write access)
- `/dev/mtdXro` – read-only character device
- `/dev/mtdblockX` – block device (used for mounting filesystems)

## Use Cases

MTD devices are essential for:

#### **1. Embedded Systems**

- Store bootloaders (e.g., U-Boot)
- Store kernel images and root filesystems
- Store configuration data and firmware

#### **2. Direct Flash Access**

- Allows low-level operations like erasing, writing, and reading flash memory
- Useful for firmware updates, diagnostics, and recovery

#### **3. Specialized Filesystems**

- Supports flash-aware filesystems like:
  - **JFFS2** (Journaling Flash File System v2)
  - **UBIFS** (UBI File System)
  - These handle wear leveling and bad block management

#### **4. Partitioning and Management**

- You can define multiple partitions for different purposes (bootloader, kernel, rootfs, data)
- Tools like `mtd-utils` help manage these partitions

## Linux kernel configuration

To enable the MTD support in Linux, add these lines to `board/efinix/ti375c529/linux/linux.config` file.

```
CONFIG_MTD=y
CONFIG_MTD_BLOCK=y
CONFIG_MTD_CHAR=y
CONFIG_MTD_NAND=y
COFNIG_MTD_NAND_ECC=y
CONFIG_MTD_SPI_NOR=y
CONFIG_MTD_SPI_NOR_USE_4K_SECTORS=y
CONFIG_MTD_SPI_NAND=y
CONFIG_MTD_BLKDEVS=y
CONFIG_MTD_NAND_CORE=y
CONFIG_MTD_UBI=y
CONFIG_MTD_PARTITIONS=y
```

## Device Tree Binding

Add this device tree binding in the `board/efinix/ti375c529/linux/linux.dts` file.

```
&spi0 {
        #address-cells = <1>;
        #size-cells = <0>;
        cmd_fifo_depth = <256>;
        rsp_fifo_depth = <256>;
        num-cs = <1>;
        status = "okay";

        flash: flash@0 {
                #address-cells = <1>;
                #size-cells = <1>;
                reg = <0>;
                compatible = "jedec,spi-nor";
                spi-max-frequency = <10000000>;

                partition@0 {
                        label = "fpga bitstream";
                        reg = <0x0 0x600000>;
                };

                partition@1 {
                        label = "opensbi";
                        reg = <0x600000 0x80000>;
                };

                partition@2 {
                        label = "u-boot";
                        reg = <0x680000 0x100000>;
                };

                partition@3 {
                        label = "user";
                        reg = <0x780000 0x40000>;
                };
        };
};
```

## Example of write and read using mtd-utils

After kernel booted up, run the `mtdinfo` command to check the mtd devices. 

```bash
root@buildroot:~# mtdinfo
Count of MTD devices:           4
Present MTD devices:            mtd0, mtd1, mtd2, mtd3
Sysfs interface supported:      yes
```

Also check the partition table of mtd device.

```bash
root@buildroot:~# cat /proc/mtd 
dev:    size   erasesize  name
mtd0: 00600000 00001000 "fpga_bitstream"
mtd1: 00080000 00001000 "opensbi"
mtd2: 00100000 00001000 "u-boot"
mtd3: 00300000 00001000 "user"
```

Step-by-step example for performing write and read back on SPI flash device using mtd-tools on partition "user" at `mtd3`.

1. Prepare a Test File.
   
   Create a small binary or text file to write:
   
   ```bash
   echo "Hello MTD!" > test_data.bin
   ```

2. Erase the MTD Device.
   
   Before writing, erase the target area:
   
   ```bash
   flash_erase /dev/mtd3 0 0
   ```
   
   This erases the entire device partition 3 (0 0 means start at offset 0 and erase all blocks).

3. Write to the MTD Device.
   
   Use `nandwrite` (for NAND) or `mtd_debug`:
   
   ```bash
   nandwrite -p /dev/mtd3 test_data.bin
   ```
   
   Or with `mtd_debug`:
   
   ```bash
   mtd_debug write /dev/mtd3 0 11 test_data.bin
   ```
   
   11 is the number of bytes in `"Hello MTD!\n"`.

4. Read Back from the MTD Device.
   
   Use `dd` or `mtd_debug` to read:
   
   ```bash
   dd if=/dev/mtd0 bs=1 count=11 of=read_back.bin
   ```
   
   Or:
   
   ```bash
   mtd_debug read /dev/mtd3 0 11 read_back.bin
   ```

5. Verify the Data Compare the original and read-back files:
   
   ```bash
   cmp test_data.bin read_back.bin && echo "Data matches!" || echo "Mismatch!"
   ```
