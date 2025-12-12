# EMMC Support

The EMMC support has been added to the Unified Hardware design for Ti375C529 devkit since Efinity 2025.2. During the booting up sequence, U-Boot would try to boot the Linux from EMMC. If the EMMC card is empty, i.e. no Linux image has been flash into it or first time use, the U-Boot would be stuck at the U-Boot terminal. You need to switch to boot from SD card first before EMMC could be used.

## Boot from EMMC

Linux would boot from emmc by default on Unified Hardware design for Ti375C529 devkit.

## Boot from SD Card

At the U-Boot terminal, run this command to boot from SD card

```
=> run sd_bootcmd
```

## Programming EMMC card with Linux image

**Prerequisite**

- Linux kernel - uImage

- Linux device tree - linux.dtb

- Root filesystem - rootfs.tar
1. Prepare the binaries files as stated in the prerequiste. You need to build the Linux image if these files does not exists. Refer to unified hardware design [README](../../boards/efinix/ti375c529/hardware/unified_hw/README.md) file for more info.

2. Boot the Linux image from SD card first before programming the EMMC card with newer Linux image.

3. After login into Linux image, setup the ethernet connection. See configure the ethernet and set a password on the [configure_the_board](../../setup/configure_the_board.md) document.

4. Create a folder in `/root/emmc` on the board. Then, copy these binaries to the folder. Replace `<user>@<host>` according to your usename and hostname. These binaries located in `/path/to/your/build_<board>/build/images`, where `<board>` is the target board such as `ti375c529`.
   
   ```
   mkdir /root/emmc
   cd /root/emmc
   scp <user>@<host>:/path/to/images/uImage .
   scp <user>@<host>:/path/to/images/linux.dtb .
   scp <user>@<host>:/path/to/images/rootfs.tar .
   ```

5. Program the emmc card with linux image.
   
   ```
   emmc_programmer /dev/mmcblk1 --no-download
   ```

6. Reboot the board once programming completed. Linux should be able to boot from emmc.
   
   ```
   reboot
   ```

7. Take out the SD card

## Steps by step to enable EMMC support

These are steps to enable EMMC support for custom design.

1. Clone this repo.

2. Configure the buildroot by appending these lines to the buildroot defconfig file in `configs/efinix_<board>_defconfig`, where `<board>` is target board to use such as `ti375c529`.
   
   ```
   BR2_PACKAGE_EMMC_PROGRAMMER=y
   BR2_TARGET_UBOOT_CONFIG_FRAGMENT_FILES="$(BR2_EXTERNAL_EFINIX_PATH)/boards/efinix/common/u-boot/uboot_32_defconfig $(BR2_EXTERNAL_EFINIX_PATH)/boards/efinix/common/u-boot/uboot_emmc_defconfig"
   BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="... $(BR2_EXTERNAL_EFINIX_PATH)/boards/efinix/common/kernel/emmc.config"
   ```

3. Run `init.sh` script to setup the buildroot environment.

4. Modify the device tree by appending the EMMC device tree node to the common DTSI file in `boards/efinix/common/dts/sapphire.dtsi`. This is example of device tree node for emmc device with memory mapped address at `0xe9300000` and interrupts number `21`. Modify the memory mapped address and interrupts number according to the design.
   
   ```
   axi4 {
       #address-cells = <1>;
       #size-cells = <1>;
       compatible = "simple-bus";
       ranges = <0x0 0xe9300000 0x110000>;
   
       emmc0: emmc@e9300000 {
           reg = <0x00000000 0x10000>, <0x00100000 0x10000>;
           compatible = "efinix,emmc-host-controller";
           interrupts = <21>;
           interrupt-parent = <&plic>;
           clocks = <&clock>;
           clock-frequency = <200000000>;
           status = "disabled";
       };
   
   };
   ```

5. Modify U-Boot Device Tree. Append the u-boot device tree node for the board specific. For example on Ti375C529, `boards/efinix/ti375c529/u-boot/uboot.dts`
   
   ```
   &emmc0 {
           compatible = "efinix,emmc-host-controller";
           reg-names = "emmc", "syscon";
           non-removable;
           no-sd;
           broken-cd;
           no-sdio;
           voltage-ranges = <1700 1950>, <2700 3600>;
           bus-width = <8>;
           clock-names = "core";
           max-frequency = <200000000>;
           max-block-size = <512>;
           cap-mmc-highspeed;
           cap-1-8v-ddr;
           mmc-hs200-1_8v;
           dma-coherent;
           keep-power-in-suspend;
           status = "okay";
   };
   ```

6. Modify Linux Device Tree. Append the linux device tree node for the board specific. For example on Ti375C529, `boards/efinix/ti375c529/linux/linux.dts`.
   
   ```
   &emmc0 {
           compatible = "efinix,emmc-host-controller";
           reg-names = "emmc", "syscon";
           non-removable;
           no-sd;
           broken-cd;
           no-sdio;
           voltage-ranges = <1700 1950>, <2700 3600>;
           bus-width = <8>;
           clock-names = "core";
           max-frequency = <200000000>;
           max-block-size = <512>;
           cap-mmc-highspeed;
           cap-1-8v-ddr;
           mmc-hs200-1_8v;
           dma-coherent;
           keep-power-in-suspend;
           status = "okay";
   };
   ```

7. Build the Linux image
   
   ```
   make
   ```

8. Program the SD card with the generated image.
