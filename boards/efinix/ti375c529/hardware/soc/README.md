# Ti375C529 Example Design

This is a defaut example design for Ti375C529 with Efinix Sapphire High Performance SoC. This design is integrated with triple speed ethernet and SD host contoller. The SD host controller and triple speed ethernet are connected to AXI slave 1 and AXI slave 2 respectively. See the table below for more details.

| Address    | Bus          | Peripheral            | Interrupt number |
| ---------- | ------------ | --------------------- | ---------------- |
| 0xe8000000 | axi slave 0  |                       |                  |
| 0xe8010000 |              | uart0                 | 1                |
| 0xe8020000 |              | i2c0                  | 3                |
| 0xe8030000 |              | spi0                  | 2                |
| 0xe8040000 |              | gpio0                 | 4, 5             |
| 0xe8100000 | apb3 slave 0 | dma0                  | 7, 8             |
| 0xe9000000 | axi slave 1  | sd host controller    | 6                |
| 0xe9100000 | axi slave 2  | triple speed ethernet | -                |

## Build Linux Image

1. Clone this repository.
   
   ```bash
   git clone https://github.com/Efinix-Inc/br2-efinix.git
   cd br2-efinix
   ```

2. Build the Linux image using preconfigure Efinity project and soc.h.
   
   ```bash
   source init.sh ti375c529 \
   boards/efinix/ti375c529/hardware/soc/soc.h -e
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
   
   - Follow [Flash firmware image](docs/flash_firmware_image.md) document for flashing the fpga bitstream, opensbi and u-boot into the Ti375C529 devkit.

6. Flash Linux image `sdcard.img` in to SD card.
   
   - you can use [Etcher](https://www.balena.io/etcher/) for Linux
   
   - or, Linux command line to flash the Linux image into SD card. See [flash linux](docs/flash_linux.md) document.

7. Access the board serial console over USB UART. See [accessing uart terminal](docs/accessing_uart_terminal.md) document.

## Customize Linux Device Tree

Use the `device_tree_generator.py` script to generate the device tree for Linux. Please note that this [soc.h](soc.h) file contain additional information regarding to axi slave which might not available on newly generated soc. For example, use this command to generate the device tree for this soc configuration.

```python
cd boards/efinix/common/sapphire-soc-dt-generator
./device_tree_generator.py \
-s config/linux_slaves.json \
-c config/linux_spi.json \
-c config/ti375_common.json \
/path/to/soc.h \
ti375c529 linux
```

**This step only require for newly generated soc.**

For customize configuration, the `soc.h` does not contain the information for `axi1` and `axi2`. Thus, user need to append the axi slave 1 (`axi1`) and axi slave 2 (`axi2`) nodes manually to the linux device tree. These nodes should be append in the `boards/efinix/common/dts/sapphire.dtsi`. 

```
{
    axi0: axi0@e8000000 {
        ...    
    };

    axi1: axi1@e9000000 {
                #address-cells = <1>;
                #size-cells = <1>;
                compatible = "simple-bus";
                ranges = <0x0 0xe9000000 0x10000>;

                mmc0: mmc@0 {
                        reg = <0x0 0x10000>;
                        compatible = "efx,sdhci";
                        interrupt-parent = <&plic>;
                        interrupts = <6>;
                        clocks = <&apb_clock 0>;
                        clock-frequency = <200000000>;
                        bus-width = <4>;
                        no-sdio;
                        no-mmc;
                        max-frequency = <100000000>;
                        status = "okay";
                };
        };

        axi2: axi2@e9100000 {
                #address-cells = <1>;
                #size-cells = <1>;
                compatible = "simple-bus";
                ranges = <0x0 0xe9100000 0x10000>;

                tsemac0: tsemac@0 {
                        reg = <0x0 0x10000>;
                        compatible = "efinix,tsemac-0.01.a";
                        clocks = <&apb_clock 0>;
                        clock-frequency = <200000000>;
                        status = "disabled";
                };
        };
}
```

Aslo, append this node to `boards/efinix/ti375c529/linux/linux.dts`.

```
&tsemac0 {
        #address-cells = <1>;
        #size-cells = <0>;
        axistream-connected = <&dma0>;
        axistream-control-connected = <&dma0>;
        device_type = "network";
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


                phy0: ethernet-phy@0 {
                        #address-cells = <1>;
                        #size-cells = <0>;
                        reg = <0>;
                        compatible = "ethernet-phy-id001c.c916";
                        device_type = "ethernet-phy";
                        reset-assert-us = <500000>;
                        reset-deassert-us = <500000>;
                };
        };

};
```
