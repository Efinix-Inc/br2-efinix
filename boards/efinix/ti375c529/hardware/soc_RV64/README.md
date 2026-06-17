# Ti375C529 Efinix Sapphire RV64 SoC Example Design

This is an example design for Ti375C529 with Efinix Sapphire RV64 SoC. In this example design, SD card is connected to the SPI interface, spi1. Refer to the table below for detailed information on the peripheral address mapping and corresponding interrupt numbers. The example design project can be found within [soc_RV64.zip](soc_RV64.zip). It includes HDL source files (VHDL/Verilog), constraint files and build scripts.

| Address    | Peripheral            | Interrupt number |
| ---------- | --------------------- | ---------------- |
| 0xe8030000 | uart0                 | 1                |
| 0xe8090000 | i2c0                  | 11               |
| 0xe8060000 | spi0                  | 6                |
| 0xe8070000 | spi1                  | 7                |
| 0xe80e0000 | gpio0                 | 16,17            |


## Build Linux Image

1. Clone this repository.
   
   ```bash
   git clone https://github.com/Efinix-Inc/br2-efinix.git
   cd br2-efinix
   ```

2. Build the RV64 Linux image using preconfigure Efinity project and soc.h.
   
   ```bash
   source init.sh ti375c529 \
   boards/efinix/ti375c529/hardware/soc_RV64/soc.h -m64
   ```

3. Build the Linux image
   
   ```bash
   make -j$(nproc)
   ```

4. The output images are located in `<path/to/br2-efinix/../build_ti375c29/build/images`.
   
   - `sdcard.img` is a Linux image
   
   - `fw_jump.bin` is an OpenSBI image
   
   - `u-boot` is an U-boot image

5. Flash firmware images.
   
   - Follow [Flash firmware image](../../../../../docs/setup/flash_firmware_image.md) document for flashing the fpga bitstream, opensbi and u-boot into the Ti375C529 devkit.

6. Flash Linux image `sdcard.img` in to SD card.
   
   - you can use [Etcher](https://www.balena.io/etcher/) for Linux
   
   - or, Linux command line to flash the Linux image into SD card. See [flash linux](../../../../../docs/setup/flash_linux.md) document.

7. Access the board serial console over USB UART. See [accessing uart terminal](../../../../../docs/setup/accessing_uart_terminal.md) document.
