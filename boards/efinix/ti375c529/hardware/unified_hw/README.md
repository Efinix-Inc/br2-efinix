# Unified Hardware

This is an example design to showcase the Efinix Sapphire High Performance SoC integrate with triple speed ethernet, SD host controller, MIPI camera and HDMI.

| Address    | Bus          | Peripheral                      | Interrupt number |
| ---------- | ------------ | ------------------------------- | ---------------- |
| 0xe8000000 | axi slave 0  |                                 |                  |
| 0xe8010000 |              | uart0                           | 9                |
| 0xe8020000 |              | i2c0                            | 10               |
| 0xe8030000 |              | spi0                            | 11               |
| 0xe8040000 |              | gpio0                           | 13, 14           |
| 0xe8100000 | apb3 slave 0 | dma0                            | 7, 8             |
| 0xe8200000 | apb3 slave 1 | camera & display registers      | -                |
| 0xe8300000 | apb3 slave 2 | hardware accelerator dma_vision | 2                |
| 0xe9000000 | axi slave 1  | sd host controller              | 6                |
| 0xe9100000 | axi slave 2  | triple speed ethernet           | -                |
| 0xe9200000 | axi slave 3  | hdmi                            | -                |

## Additional Hardware Requirements

- Raspberry Pi Camera V3 Module

- MIPI daughter card

- HDMI daughter card

## Board Setup

1. Connect a USB cable type C to Ti375C529 development board and to your computer.

2. Insert micro SD card into the SD card slot.

3. Attach the network cable into the board.

4. Attach the Raspberry Pi Camera V3 module to MIPI daughter card. Then, attach it to port P2.

5. Attach the HDMI daughter card to port P1.

## Build Linux Image

Steps to buid the Linux image for unified hardware design.

1. Clone this repository.
   
   ```bash
   git clone http://gitlab/mnalim/br2-efinix
   cd br2-efinix
   ```

2. Build the Linux image using preconfigure Efinity project and soc.h.
   
   ```bash
   source init.sh ti375c529 \
   boards/efinix/ti375c529/hardware/unified_hw/soc.h \
   -p -u
   ```

3. Build the Linux.
   
   ```bash
   make -j$(nproc)
   ```

4. The output images are located in `<path/to/br2-efinix/../build_ti375c529/build/images`.
   
   - `sdcard.img` is a Linux image
   
   - `fw_jump.bin` is an OpenSBI image
   
   - `u-boot` is an U-boot image

5. Flash firmware images.
   
   - Follow [Flash firmware image](docs/flash_firmware_image.md) document for flashing the fpga bitstream, opensbi and u-boot into the Ti375C529 devkit.

6. Flash Linux image `sdcard.img` in to SD card.
   
   - you can use [Etcher](https://www.balena.io/etcher/) for Linux
   
   - or, Linux command line to flash the Linux image into SD card. See [flash linux](docs/flash_linux.md) document.

7. Access the board serial console over USB UART. See [accessing uart terminal](docs/accessing_uart_terminal.md) document.

## Demo

See [evsoc_camera](../../../../../package/evsoc_camera/README.md) tutorial.
