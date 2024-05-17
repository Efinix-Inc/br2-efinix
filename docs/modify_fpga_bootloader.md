# Modify FPGA Bootloader

> You are require to modify the bootloader if generate your own custom Sapphire SoC configuration.

The bootloader is the first program get executed when the FPGA board power on. The default bootloader would not able to boot Linux kernel. 

In this tutorial, we are going to modify the bootloader program so that it can boot Linux. OpenSBI and U-Boot will be loaded first by this bootloader program before it can boot Linux.

> `$EFINITY_PROJECT` is the path to the current Efinity project. You may either replace the `$EFINITY_PROJECT` with the absolute path to your current Efinity Project (i.e. /home/<username>/efinity/2022.M/project/linux_test/soc/ip/soc1).

## Prerequsite

### Install Efinity RISC-V Embedded Software IDE

Make sure to install [Efinity RISC-V Embedded Software IDE](https://www.efinixinc.com/support/ip/riscv-sdk.php) on your development machine. The RISC-V IDE provide RISC-V toolchain, debugger and Eclipse IDE. Refer to [RISC-V SoC Hardware and Software User Guide](https://www.efinixinc.com/support/docsdl.php?s=ef&pn=SAPPHIREUG) to install the IDE.

## Firmware Address

The table below shows the SPI flash address and corresponding RAM address for each firmware. The bootloader program will load these firmware to designated address during the bootstraping process.

| Firmware | SPI Flash Address | RAM Address |
| -------- | ----------------- | ----------- |
| OpenSBI  | 0x00600000        | 0x01000000  |
| U-Boot   | 0x00680000        | 0x01040000  |

## Part 1: Modified Bootloader Program

### Using script

Modifying bootloader program can be challenging for beginner. You can use `modify_bootloader.sh` script to simplify the steps for modifying bootloader program. The script will handle compiling bootloader and generating the memory initialization files. The `modify_bootloader.sh`  script located in `br2-efinix/boards/efinix/common`.

```bash
This script used to modify bootloader program for booting Linux.

command
./modify_bootloader.sh <devkit> <efinity project directory> <RISCV IDE directory> <Efinity installation directory>

supported <devkit> are t120f324, ti60f225
<Efinity project directory> can be /home/<user>/soc/ip/soc1
<RISCV IDE> can be /home/<user>/efinity/efinity-riscv-ide-2022.2.3
<Efinity installation directory> can be /home/<user>/efinity/2022.1/bin

Example command for t120f324 devkit
./modify_bootloader.sh \
t120f324 \
/home/<user>/soc/ip/soc1 \
/home/<user>/efinity/efinity-riscv-ide-2022.2.3 \
/home/<user>/efinity/2022.1/bin
```

### Modify manually

1. Create a Efinity project in `/home/user/efinity_projects` as `EFINITY_PROJECT`. See [Generate Sapphire SoC for Linux](generate_sapphire_soc_for_linux.md) guide to generate a Sapphire SoC.

2. Copy and replace [bootloaderConfig.h](board/efinix/common/bootloaderConfig.h) to your project path. For example, `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/bsp/efinix/EfxSapphireSoc/app/bootloaderConfig.h`.

3. If the target SoC is multicore, then enable SMP flag in `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/bsp/efinix/EfxSapphireSoc/include/soc.mk` . Just append it in `soc.mk`.
   
   ```makefile
   CFLAGS += -DSMP
   ```

4. Compile the bootloader program. See secton Part 2: Compile Bootloader Program.

## Part 2: Compile Bootloader Program

Bootloader program is located in `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/software/standalone/bootloader`. There are **TWO** ways to compile the bootloader program.

### Method 1: Using Efinity RISC-V Embedded Software IDE

1. Run Efinity RISC-V Embedded Software IDE from terminal.
   
   ```bash
   cd </path/to/efinity/efinity-riscv-ide-2022.2.3
   ./efinity-riscv-ide
   ```

2. At the Efinity RISC-V IDE Launcher, click `Browse` button. Select the folder, `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>` as the workspace.
   
   > The workspace must be placed in this directory in order for the Efinity RISC-V IDE toolchain to use the correct bsp files.

3. Click on `File` -> `Import` -> `Efinix Projects`->`Efinix Makefile Project` then click `Next`.

4. Put the `BSP Location`. The location could be `$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/bsp` then click `Next`.

5. Select `bootloader` then click `Finish`.

6. Right click on the `bootloader` on the `Project Explorer` and select `Build Project`.

### Method 2: Using Command Line in Linux

```bash
export PATH=</path/to/efinity/efinity-riscv-ide-2022.2.3/toolchain/bin:$PATH
cd $EFINITY_PROJECT/T120F324_devkit/embedded_sw/<Project Name>/software/standalone/bootloader
BSP_PATH=$EFINITY_PROJECT/T120F324_devkit/embedded_sw/<project name>/bsp/efinix/EfxSapphireSoc make
```

## Part 3: Compile Efinity Project

1. Open the existing Efinity project using Efinity software.

2. Regenerate the Sapphire SoC configuration if the project already exitst.
   
   a) For **Sapphire SoC**
   
   - Right click on the `Sapphire IP -> configure`.
     
     ![](img/sapphire_configure.png)
   
   - On the IP configuration of Sapphire SoC, goto `Cache/Memory` tab.
     
     ![](img/on-chip-ram.png)
     
     - Enable `Custom On-Chip RAM Application`.
     
     - Give the full path to the `bootloader.hex` at `User Application Path`.
   
   - Click `Generate` button to regenerate the SoC configuration with modified bootloader.
   
   - Compile the Efinity project.
   
   b) For **Ti375C529 with Harden RISC-V Sapphire SoC**
   
   - Open `Efinity Interface Designer`.
     
     ![](img/on-chip-ram-harden-soc.png)
   
   - At `Design Explorer`, select `Quad-Core RISC-V`.
   
   - Go to `Base` tab on the `Block Editor`, then browse the `bootloader.hex` at the `On-Chip Ram Configuration File`. Click `Design -> Check Design` and click `Design -> Generate Efinity Constraint File`.
   
   - Then, click `Generate` bitstream.
     
     > Please note that you are not require to compile the whole project after modify the bootloader.
