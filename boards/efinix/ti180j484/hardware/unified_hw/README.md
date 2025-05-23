# Unified Hardware Design for Sapphire SoC

This is an example design to showcase the Efinix Sapphire SoC integrate with

- triple speed ethernet (10/100/1000 Mbps)

- SD host controller

- MIPI camera

- HDMI display

Project repository: [EmbeddedSystem-Solution](https://github.com/Efinix-Inc/EmbeddedSystem-Solution.git) branch Ti180J484

## Mofidy the bootloader of unified hardware design

This section is mandatory for boot up Linux using the unified hardware design.

1. Clone the hardware design repository
   
   ```bash
   cd ~
   git clone https://github.com/Efinix-Inc/EmbeddedSystem-Solution.git -b Ti180J484
   cd EmbeddedSystem-Solution
   ```

2. Modify the bootloader using `modify_bootloader.sh` script. Please note that `modify_bootlader.sh` require 3 arguments. See [modify bootloader](docs/modify_fpga_bootloader.md) guide for more details. Here is an example to modify the bootloader.
   
   ```bash
   cd ~/br2-efinix/boards/efinix/common
   ./modify_bootloader.sh \
   ti180j484 \
   ~/ \
   ~/efinity/efinity-riscv-ide-2023.2.5.1
   ```

## Address Mapping

See [Address Mapping for Sapphire SoC](https://github.com/Efinix-Inc/EmbeddedSystem-Solution/blob/Ti180J484/docs/soc/addr_mapping_soc.md) document.

## Board Setup

See [Setup Developement Board: Titanium Ti180J484](https://github.com/Efinix-Inc/EmbeddedSystem-Solution/blob/Ti180J484/docs/hardware/setup_devkit_Ti180J484.md)

## Build Linux Image

Steps to buid the Linux image for unified hardware design.

1. Clone this repository.
   
   ```bash
   git clone https://github.com/Efinix-Inc/br2-efinix.git
   cd br2-efinix
   ```

2. Build the Linux image using preconfigure Efinity project and soc.h.
   
   ```bash
   source init.sh ti180j484 \
   boards/efinix/ti180j484/hardware/unified_hw/soc_ti180.h -u
   ```

3. Build the Linux.
   
   ```bash
   make -j$(nproc)
   ```

4. The output images are located in `<path/to/br2-efinix/../build_ti180j484/build/images`.
   
   - `sdcard.img` is a Linux image
   
   - `fw_jump.bin` is an OpenSBI image
   
   - `u-boot` is an U-boot image

5. Flash firmware images.
   
   - Follow [Flash firmware image](../../../../../docs/flash_firmware_image.md) document for flashing the fpga bitstream, opensbi and u-boot into the Ti180J484 devkit.

6. Flash Linux image `sdcard.img` in to SD card.
   
   - you can use [Etcher](https://www.balena.io/etcher/) for Linux
   
   - or, Linux command line to flash the Linux image into SD card. See [flash linux](../../../../../docs/flash_linux.md) document.

7. Access the board serial console over USB UART. See [accessing uart terminal](../../../../../docs/accessing_uart_terminal.md) document.

## Demo

See [evsoc_camera](../../../../../package/evsoc_camera/README.md) tutorial.
