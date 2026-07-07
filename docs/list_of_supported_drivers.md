# List of Supported Drivers

## Linux v6.6 Host Drivers

| #   | Driver      | RV32 | RV64 | Driver File                                                           |
| --- | ----------- | ---- | ---- | --------------------------------------------------------------------- |
| 1   | UART        | Y    | Y    | patches/linux/6.6/0001-driver-tty-add-serial-spinal-uart-driver.patch |
| 2   | SPI         | Y    | Y    | patches/linux/6.6/0002-drivers-spi-add-spi-spinal-lib-driver.patch    |
| 3   | GPIO        | Y    | Y    | patches/linux/6.6/0003-drivers-add-spinal-lib-gpio-driver.patch       |
| 4   | I2C         | Y    | Y    | kernel_modules/i2c/src/i2c.c                                          |
| 5   | SDHC        | Y    | N    | patches/linux/6.6/0005-drivers-mmc-add-Efinix-sdhci-driver.patch      |
| 6   | EMMC        | Y    | N    | patches/linux/6.6/0008-mmc-Add-Efinix-eMMC-driver.patch               |
| 7   | DMA         | Y    | N    | patches/linux/6.6/0009-drivers-add-Efinix-DMA-driver.patch            |
| 8   | Ethernet    | Y    | N    | patches/linux/6.6/0006-drivers-add-efinix-tsemac-driver.patch         |
| 9   | Framebuffer | Y    | N    | patches/linux/6.6/0010-fb-add-Efinix-framebuffer-driver.patch         |
| 10  | Watchdog    | Y    | Y    | patches/linux/6.6/0007-drivers-add-efinix-watchdog-driver.patch       |
| 11  | EVSoC       | N    | N    | kernel_modules/evsoc/src/evsoc_main.c                                 |

### Linux drivers RV32/64 vs Devkits

Notation: <RV32/RV64>, Y = Yes, N = No

| #   | Driver      | Ti375N1156 | Ti375C529 | Ti180J484 | T120F324 |
| --- | ----------- | ---------- | --------- | --------- | -------- |
| 1   | UART        | Y/N        | Y/Y       | Y/N       | Y/N      |
| 2   | SPI         | Y/N        | Y/Y       | Y/N       | Y/N      |
| 3   | GPIO        | Y/N        | Y/Y       | Y/N       | Y/N      |
| 4   | I2C         | Y/N        | Y/Y       | Y/N       | Y/N      |
| 5   | SDHC        | Y/N        | Y/N       | Y/N       | N/N      |
| 6   | EMMC        | N/N        | Y/N       | Y/N       | N/N      |
| 7   | DMA         | Y/N        | Y/N       | Y/N       | N/N      |
| 8   | Ethernet    | Y/N        | Y/N       | Y/N       | N/N      |
| 9   | Framebuffer | N/N        | Y/N       | Y/N       | N/N      |
| 10  | Watchdog    | Y/N        | Y/Y       | Y/N       | N/N      |
| 11  | EVSoC       | N/N        | N/N       | N/N       | N/N      |

## U-Boot v2025.01 Host Drivers

| #   | Driver   | RV32 | RV64 | Driver File                                                                  |
| --- | -------- | ---- | ---- | ---------------------------------------------------------------------------- |
| 1   | UART     | Y    | Y    | patches/uboot/2025.01/0002-drivers-add-serial-spinal-driver.patch            |
| 2   | SPI      | Y    | Y    | patches/uboot/2025.01/0003-drivers-add-vexriscv_spi-driver.patch             |
| 3   | SDHC     | Y    | N    | patches/uboot/2025.01/0004-mmc-add-efinix-sd-host-driver.patch               |
| 4   | EMMC     | Y    | N    | patches/uboot/2025.01/0007-mmc-Add-Efinix-eMMC-driver.patch                  |
| 5   | Ethernet | Y    | N    | patches/uboot/2025.01/0011-driver-net-add-efinix-tsemac-driver-support.patch |
| 6   | Watchdog | Y    | Y    | patches/uboot/2025.01/0005-drivers-add-u-boot-efinix-watchdog-driver.patch   |

### U-Boot driver RV32/64 vs Devkits

Notation: <RV32/RV64>, Y = Yes, N = No

| #   | Driver   | Ti375N1156 | Ti375C529 | Ti180J484 | T120F324 |
| --- | -------- | ---------- | --------- | --------- | -------- |
| 1   | UART     | Y/N        | Y/Y       | Y/N       | Y/N      |
| 2   | SPI      | Y/N        | Y/Y       | Y/N       | Y/N      |
| 3   | SDHC     | Y/N        | Y/N       | Y/N       | N/N      |
| 4   | EMMC     | N/N        | Y/N       | N/N       | N/N      |
| 5   | Ethernet | Y/N        | Y/N       | Y/N       | N/N      |
| 6   | Watchdog | Y/N        | Y/Y       | Y/N       | N/N      |
