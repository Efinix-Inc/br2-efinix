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
This step will build Linux based on Buildroot 2020.05.3

1. Create a working directory.
```
mkdir ~/workspace
cd workspace
mkdir build
```

2. Clone this repo and Buildroot in `workspace` directory.
```
git clone http://gitlab/mnalim/br2-efinix -b dev-next
git clone https://github.com/buildroot/buildroot.git -b 2021.05.3
```

3. Go to `build` directory created in step 1.

4. Configure the Buildroot to build the Linux image for T120T324 board `efinix_t120f324_defconfig`.  
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

OpenSBI and U-boot can be flash on the supported board using Efinix Efinity programmer and FTDI USB UART cable.  
You need to download Efinix Efinity on Efinity official website.  
- [Linux](https://www.efinixinc.com/support/downloads-license.php?platform=linux&os=ubuntu&v=2021.2.323)  
- [Windows](https://www.efinixinc.com/support/downloads-license.php?platform=windows&os=windows&v=2021.2.323)  

| Name | Binary | SPI flash address |
| ------ | ------ |------ |
| OpenSBI | fw_jump.bin | 0x00400000 |
| U-boot | u-boot.bin | 0x00480000|

To flash the OpenSBI and U-boot, follow the following steps
1. Download the Efinix Efinix and install it. Follow the official [documentation](https://www.efinixinc.com/docs/efinity-installation-v2.6.pdf) on installation process.

2. Open the Efinity program.
```
cd efinity/<version>/bin
./setup.sh 
efinity
```

3. Click the `programmer` icon on the Efinity to launch the `programmer`.

4. At the `Image` section on `programmer`, click **Combine Multiple Image Files** to select FPGA bitstream (Sapphire SoC), OpenSBI and U-boot images.

5. Select `Mode` and choose **Generic Image Combination**. Then give the name of `Output File`. The `Output File` is name of combination file which consist of FPGA bitstream, OpenSBI and U-boot images.

6. Click `Add Image` to add FPGA bitstream, OpenSBI and U-boot images. Then set the `Flash Address` for each image file.

7. The table should look like this

| Flash Address | Flash Length | Image File |
| ------ | ------ |------ |
| soc.hex | | 0x0|
| fw_jump.bin | | 0x00400000 |
| u-boot.bin | | 0x00480000|

8. Click **Apply** to save the output image.

9. On `programmer` interface at section `Programming Mode`, select `SPI Active` then click icon **Start Program**. It will takes some time to program the SPI flash.


## Flash Linux image on SD card

You need at least 1GB of SD card. To flash Linux image on SD card, you need privilege mode by using `sudo` command.  
`sdX` is your SD card. `X` can be any number.

```
cd $HOME/workspace/build/images
sudo dd if=sdcard.img of=/dev/sdX
```
on Windows, you can use [Etcher](https://www.balena.io/etcher/) to flash the Linux image on SD card.

## Documentation

1. [Buildroot documentation](https://buildroot.org/docs.html).  
2. [RISCV Sapphire SoC datasheet](https://www.efinixinc.com/docs/riscv-sapphire-ds-v2.0.pdf)  
3. [Others documentation](http://gitlab/mnalim/br2-efinix/-/tree/dev-next/docs)

## Supported Board

Currently supported board as follows  
1. Trion T120F324  
2. Titanium Ti60F225  

## License

This project is licensed under the [GPLv2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later.  
Buildroot is licensed under the [GPLv2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later with exceptions.
