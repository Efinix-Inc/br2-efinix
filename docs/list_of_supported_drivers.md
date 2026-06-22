# List of Supported Drivers

## Linux v5.10 Host Drivers

| #   | Driver      | RV32 | Driver File                                                                        |
| --- | ----------- | ---- | ---------------------------------------------------------------------------------- |
| 1   | UART        | Y    | patches/linux/v5.10/0003-drivers-add-serial-spinal-driver.patch                    |
| 2   | SPI         | Y    | patches/linux/v5.10/0001-drivers-add-spi-spinal-driver.patch                       |
| 3   | GPIO        | Y    | patches/linux/v5.10/0002-drivers-add-gpio-spinal-driver.patch                      |
| 4   | I2C         | Y    | kernel_modules/i2c/src/i2c.c                                                       |
| 5   | SDHC        | Y    | patches/linux/v5.10/0014-mmc-add-efinix-sdhci-driver.patch                         |
| 6   | EMMC        | Y    | patches/linux/v5.10/0024-mmc-add-efinix-emmc-driver.patch                          |
| 7   | DMA         | Y    | patches/linux/v5.10/0020-dma-add-Efinix-DMA-controller.patch                       |
| 8   | Ethernet    | Y    | patches/linux/v5.10/0013-drivers-add-efinix-tsemac-driver.patch                    |
| 9   | Framebuffer | Y    | patches/linux/v5.10/0021-fb-add-Efinix-framebuffer-driver.patch                    |
| 10  | Watchdog    | Y    | patches/linux/v5.10/0023-drivers-add-efinix-watchdog-driver.patch                  |
| 11  | SDIO        | Y    | patches/linux/v5.10/0026-Add-a-new-SDIO-controller-driver-for-Efinix-devices.patch |
| 12  | EVSoC       | Y    | kernel_modules/evsoc/src/evsoc_main.c                                              |

## U-Boot v2021.04 Host Drivers

| #   | Driver   | RV32 | Driver File                                                                |
| --- | -------- | ---- | -------------------------------------------------------------------------- |
| 1   | UART     | Y    | patches/uboot/2021.04/0003-driver-add-vexriscv-serial-driver.patch         |
| 2   | SPI      | Y    | patches/uboot/2021.04/0002-drivers-add-vexriscv_spi-driver.patch           |
| 3   | SDHC     | Y    | patches/uboot/2021.04/0006-mmc-add-efinix-sd-host-driver.patch             |
| 4   | EMMC     | Y    | patches/uboot/2021.04/0013-mmc-add-efinix-emmc-driver.patch                |
| 5   | Ethernet | Y    | patches/uboot/2021.04/0014-drivers-add-efinix-ethernet-tse-driver.patch    |
| 6   | Watchdog | Y    | patches/uboot/2021.04/0012-drivers-add-u-boot-efinix-watchdog-driver.patch |
