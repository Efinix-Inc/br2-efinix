# Efinix Buildroot External

Buildroot external tree for building Linux on Sapphire SoC.
It include OpenSBI, U-boot, Linux, Buildroot configuration files
and some patches. This project has customize configurations to support Efinix Sapphire SoC.

## System Dependencies

Tested on Ubuntu 18.04 LTS
```
sudo apt-get update && sudo apt-get upgrade -y
sudo apt-get install -y sed make binutils build-essential bash patch gzip \
bzip2 perl tar cpio unzip rsync file bc wget autoconf automake libtool
```

## Buildroot Dependencies

This repo require specific version of Buildroot to work with.
See the VERSION file that contain the Buildroot version to use.
The version of Buildroot must be matched with this repo version.

## Build

To build Linux image, OpenSBI and U-boot follow the steps below.
This step will build Linux based on Buildroot 2020.05.

1. Create a working directory.
```
mkdir ~/workspace
cd workspace
mkdir build
```

2. Clone this repo and Buildroot in `workspace` directory.
```
git clone http://gitlab/mnalim/br2-efinix -b 2020.05
git clone https://github.com/buildroot/buildroot.git -b 2020.05
```

3. Go to `build` directory created in step 1.

4. Configure the Buildroot to build the `efinix_t120f324_defconfig`.  
   `<path/to/br2-external>` is `$HOME/workspace/br2-efinix`  
   `<path/to/buildroot>` is `$HOME/workspace/buildroot`  
   `<target_buildroot_defconfig>` is `efinix_t120f324_defconfig`

```
make O=$PWD BR2_EXTERNAL=<path/to/br2-external> -C <path/to/buildroot> <target_buildroot_defconfig>
```

Run this command. It will takes a while to compile.  
In a mean time you can drink some coffee :D
```
make O=$PWD BR2_EXTERNAL=$HOME/workspace/br2-efinix -C $HOME/workspace/buildroot efinix_t120f324_defconfig
make -j$(nproc)
```

5. The output is located in `$HOME/workspace/build/images`.  
    `sdcard.img` is a Linux image   
    `fw_jump.bin` is an OpenSBI image  
    `u-boot` is an U-boot image  

## Flash firmware image

OpenSBI and U-boot can be flash on the supported board using OpenOCD and FTDI USB UART cable.
You need to download Efinix SDK which come with OpenOCD configuration.  
- [Linux]()  
- [Windows]()  

| Name | Binary | SPI flash address |
| ------ | ------ |------ |
| OpenSBI | fw_jump.bin | 0x00400000 |
| U-boot | u-boot.bin | 0x00480000|

To flash the firmware (OpenSBI and U-boot) follow the following steps
1. Download the Efinix SDK and unzip it to `$HOME` directory.

```
cd $HOME
unzip SDK_Ubuntu.zip
```

2. Clone Sapphire BSP.

```
git clone <t120f324_soc>
```

3. Run OpenOCD.

```
cd <t120f324_soc>
SDK_Ubuntu/openocd/build-x86_64/bin/openocd \
-f bsp/efinix/EfxSapphireSoc/openocd/ftdi.cfg \
-c "set CPU0_YAML cpu0.yaml" \
-f bsp/efinix/EfxSapphireSoc/openocd/flash.cfg
```

4. Open another terminal to connect to telnet session on port `4444`.

```
telnet localhost 4444
```

5. Flash the OpenSBI.

```
> flash write_image erase unlock /path/to/fw_jump.bin 0x00400000
```

6. Flash the U-boot.

```
> flash write_image erase unlock /path/to/u-boot.bin 0x00480000
```

## Flash Linux image on SD card

You need at least 1GB of SD card. To flash Linux image on SD card, you need privilege mode by using `sudo` command.  
`sdX` is your SD card. `X` can be any number.

```
cd $HOME/workspace/build/images
sudo dd if=sdcard.img of=/dev/sdX
```
on Windows, you can use [Etcher](https://www.balena.io/etcher/) to flash the Linux image on SD card.

## Documentation

1. [buildroot documentation](https://buildroot.org/docs.html).  
2. [RISCV Sapphire SoC datasheet](https://www.efinixinc.com/docs/riscv-t120f324-ds-v2.0.pdf)

## Supported Board

Currently supported board as follows  
1. Trion t120f324

## License

This project is licensed under the [GPLv2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later.  
Buildroot is licensed under the [GPLv2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later with exceptions.
