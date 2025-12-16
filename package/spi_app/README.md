# SPI App

This is an example of Linux user space SPI application for reading and writing to SPI flash device. It use spidev framework for controlling SPI device from Linux user space. The `spidev`is the interface between the user and the peripheral exposed by the Linux kernel. It will appear such as`/dev/spidev0.0`. It needs to be enabled through the kernel configuration and device tree.

## SPI App Usage

```bash
spi_app -h
help
-a      Address to read/write of SPI flash
-d      Set SPI device name. Example, /dev/spidev0.0
-l      Length of data to read in byte
-s      Set SPI clock frequency in hertz
-r      Read mode
-w      Write mode
-m      Message to write to SPI flash
-v      Show verbose debug message
-h      show this message

Example usage
Reading SPI flash
spi_app -r -a 0x800000 -l 4

Writing to SPI flash
spi_app -w -a 0x800000 -m "hello world"
```

### Read from SPI flash device

Reading from SPI flash at address 0x680000 with 64 bytes of length

```bash
spi_app -a 0x680000 -r -l 64

Reading SPI Flash

 Address:  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
0x680000: 13 02 05 00 93 84 05 00 93 01 00 00 97 72 07 00 
0x680010: 83 a2 02 94 73 90 52 10 73 10 40 10 93 02 00 ff 
0x680020: 37 03 04 01 33 71 53 00 13 05 01 00 ef f0 00 5e 
0x680030: 13 04 05 00 13 01 05 00 97 72 07 00 83 a2 02 a3 
```

### Write to SPI flash device

Write some data to SPI flash device at address 0x800000.

```bash
spi_app -a 0x800000 -w -m "hello world"

Writing to SPI flash at 0x800000
68 65 6c 6c 6f 20 77 6f 72 6c 64
```

#### Verify the write data on SPI flash

```bash
spi_app -a 0x800000 -r

Reading SPI Flash

 Address:  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
0x800000: 68 65 6c 6c 6f 20 77 6f 72 6c 64 ff ff ff ff ff
```

> Please note that writing at address range less than 0x800000 of SPI flash will result of unable to boot up the Linux after reset due to corrupted the Linux bootloader. See [Firware Address](../../docs/setup/modify_fpga_bootloader.md) to learn more.



## Kernel Configuration

Enable spidev support in the kernel configuration to use spidev framework. For example to enable spidev configuration for Ti375C529 devkit at `boards/efinix/ti375c529/linux/linux.config` by append this line.

```
CONFIG_SPI_SPIDEV=y
```

## Device Tree

Add `flash` device tree node to `spi0` node in `linux.dts` file. For example using Ti375C529 is in `boards/efinix/ti375c529/linux/linux.dts`. The `dummy-cycle` is require for SPI flash device. Please note that this property will not work for SPI SD card.

```
&spi0 {
        #address-cells = <1>;
        #size-cells = <0>;
        cmd_fifo_depth = <256>;
        rsp_fifo_depth = <256>;
        num-cs = <1>;
        dummy-cycle;
        status = "okay";

        flash: flash@0 {
                #address-cells = <1>;
                #size-cells = <1>;
                reg = <0>;
                compatible = "rohm,dh2228fv";
                spi-max-frequency = <6000000>;
        };
};
```

## How to build this application

1. Clone the repo.

2. Enable this package on device configuration. For example on Ti375C529 devkit,
   
   add this line to `configs/efinix_ti375c529_defconfig`.
   
   ```
   BR2_PACKAGE_SPI_APP=y
   ```

3. Run `int.sh` script.

4. Add the spidev kernel configuration

5. Add child node of flash to spi0 in Linux device tree

6. Build the Linux image.
