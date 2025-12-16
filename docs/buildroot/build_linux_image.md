# Build Linux Image

Please note that you need to generate the SoC first before proceed with this section. Follow these steps to build Linux image, OpenSBI and U-boot for Ti180J484 development kit. These steps are applicable for all supported devices.

1. Clone this repository.
   
   ```bash
   git clone https://github.com/Efinix-Inc/br2-efinix
   cd br2-efinix
   ```

2. If you are using preconfigure Efinity project for Ti180J484 with 4 cores, then point the path to `soc.h` as follows
   
   ```bash
   source init.sh ti180j484 \
   boards/efinix/ti180j484/hardware/multicores/soc4/embedded_sw/SapphireCore/bsp/efinix/EfxSapphireSoc/include/soc.h
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
   
   - Follow [Flash firmware image](../setup/flash_firmware_image.md) document for flashing the fpga bitstream, opensbi and u-boot into the Ti180J484 devkit.

6. Flash Linux image `sdcard.img` in to SD card.
   
   - you can use [Etcher](https://www.balena.io/etcher/) for Linux
   
   - or, Linux command line to flash the Linux image into SD card. See [flash linux](../setup/flash_linux.md) document.

7. Access the board serial console over USB UART. See [accessing uart terminal](../setup/accessing_uart_terminal.md) document.

8. Example of Linux boot.
   
   ![alt text](../img/boot_linux_smp.gif)

9. Use the login prompt as `root` at linux prompt.
