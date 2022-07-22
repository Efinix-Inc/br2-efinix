# Modify FPGA Bootloader

The bootloader is the first program get executed when the FPGA board power on. For the Sapphire SoC, the bootloader will load the program at designated location as specify in `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/bsp/efinix/EfxSapphireSoc/app/bootloaderConfig.h`. By default, the bootloader will load user software binary from address `0x00380000` in  the SPI flash memory. Then it will copy the user software binary to RAM at address `0x00001000`.

In this tutorial, we are going to modify the bootloader program so that it can boot Linux. OpenSBI and U-Boot will be loaded first by this bootloader program before it can boot Linux.
> `$EFINITY_PROJECT` is the path to the current Efinity project. You may either replace the `$EFINITY_PROJECT` with the absolute path to your current Efinity Project(i.e. /home/<username>/efinity/2022.M/project/linux_test/soc/ip/soc1) or you may add the path in .bashrc file(only for linux) as below;
>>> #custom path to eclipse project 
>>> export EFINITY_PROJECT="/home/jjho/efinity/2022.M/project/linux_test/soc/ip/soc1"
## Prerequsite

### Install RISC-V SDK

Make sure to install [RISC-V SDK](https://www.efinixinc.com/support/ip/riscv-sdk.php) on your development machine. The RISC-V SDK provide RISC-V toolchain, debugger and Eclipse. Refer to [RISC-V SoC Hardware and Software User Guide](https://www.efinixinc.com/docs/riscv-sapphire-ug-v3.2.pdf) to install the SDK.

## Firmware Address

The table below shows the SPI flash address and corresponding RAM address for each firmware. The bootloader program will load these firmware to designated address during the bootstraping process.

| Firmware | SPI Flash Address | RAM Address |
| -------- | ----------------- | ----------- |
| OpenSBI  | 0x00400000        | 0x01000000  |
| U-Boot   | 0x00480000        | 0x01040000  |

## Modified Bootloader Program

This is an example of modified `bootloaderConfig.h` for booting Linux. This file is located in `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/bsp/efinix/EfxSapphireSoc/app/bootloaderConfig.h`. You may copy and replace it.

```c
#pragma once

#include "bsp.h"
#include "io.h"
#include "spiFlash.h"
#include "riscv.h"

#define SPI SYSTEM_SPI_0_IO_CTRL
#define SPI_CS 0

#define OPENSBI_MEMORY    0x01000000
#define OPENSBI_FLASH     0x00400000
#define OPENSBI_SIZE      0x040000

#define UBOOT_MEMORY      0x01040000
#define UBOOT_SBI_FLASH   0x00480000
#define UBOOT_SIZE        0x0C0000

void bspMain()
{
#ifndef SPINAL_SIM
    spiFlash_init(SPI, SPI_CS);
    spiFlash_wake(SPI, SPI_CS);
    bsp_putString("OpenSBI copy\r\n");
    spiFlash_f2m(SPI, SPI_CS, OPENSBI_FLASH, OPENSBI_MEMORY, OPENSBI_SIZE);
    bsp_putString("U-Boot copy\r\n");
    spiFlash_f2m(SPI, SPI_CS, UBOOT_SBI_FLASH, UBOOT_MEMORY, UBOOT_SIZE);
#endif

    bsp_putString("Payload boot\r\n");
    void (*userMain)(u32, u32, u32) = (void (*)(u32, u32, u32))OPENSBI_MEMORY;
    #ifdef SMP
    smp_unlock(userMain);
    #endif
    userMain(0, 0, 0);
}
```

## Compile Bootloader Program

Bootloader program is located in `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/software/standalone/bootloader`. There are two ways to compile the bootloader program.

### Using Eclipse

1. Run Eclipse from terminal.
   
   ```bash
   cd </path/to/SDK_Ubuntu>/SDK_Ubuntu
   source setup.sh
   ./run_eclipse.sh
   ```

2. Then, select 1 to use SapphireSoC configuration to launch Eclipse.
3. At the Eclipse IDE Launcher, click `Browse` button. Select the folder, `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>` as the workspace. 
>The workspace must be placed in this directory in order for the Eclipse toolchain to use the correct bsp files. 


3. Click on `File` -> `New` -> `MakeFile Project with Existing Code`. Select `Browse` to find the bootloader project in `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/software/standalone/bootloader`. Leave the `Toolchain for indexer Settings` as `<none>`. Then click `Finish`.

4. Right click on the `bootloader` on the `Project Explorer` and select `Build Project`.

### Using Command Line in Linux

```bash
export PATH=</path/to/SDK_Ubuntu>/SDK_Ubuntu/riscv-xpack-toolchain_8.3.0-2.3_linux/bin:$PATH
cd $EFINITY_PROJECT/T120F324_devkit/embedded_sw/<Project Name>/software/standalone/bootloader
BSP_PATH=$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/bsp/efinix/EfxSapphireSoc
make
```

## Generate Memory Initialization Files

The memory initialization binary files are named as  `EfxSapphireSoc.v_toplevel_system_ramA_logic_ram_symbol*.bin`. These files automatically generated during the soc generation. These files need to be regenerated when the bootloader program gets modified. 

1. Use `binGen.py` script to regenerate the files which can be found in `$EFINITY_PROJECT/embedded_sw/<project name>/tool/binGen.py`.
   
   Example command.
   
   ```bash
   cd $EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/tool
   python3 binGen.py -f 1 -s 4096 -b ../software/standalone/bootloader/build/bootloader.bin
   ```
   
   
>   where,
>   -f, fpu. If enable set to 1. Else set to 0
>   -s, On-Chip RAM size. You can get this value at parameter "--onChipRamSize"
>    in $EFINITY_PROJECT/source/soc_config. The value must be in decimal. By default
>    the value is 4096(0x1000 in hexadecimal).
>   -b, path of bootloader.bin
   

2. Copy the generated memory initialization bin files
   
   ```bash
   cp -r $EFINITY_PROJECT/T120F324_devkit/embedded_sw/<Project Name>/tool/rom/*.bin \
   $EFINITY_PROJECT/T120F324_devkit
   ```
