# Generate Sapphire High Performance SoC

## Prerequsite

- Efinity Software 2023.2 with patch 5 or later

- Efinity RISCV IDE 2023.2 or later

## Install Efinity Software

Follow the official [document](https://www.efinixinc.com/support/docsdl.php?s=ef&pn=UG-EFN-INSTALL) for installing Efinity software.

## Part 1: Generate Sapphire High Performance SoC

1. Launch Efinity Software.

2. Create a new project. Select `File -> Create Project`. On `Project Editor` window, set the following then click `OK`.
   
   - Name: `soc`
   
   - Location: `$EFINITY_HOME/project/soc`
   
   - Description:  (optional) `Example design for Ti375C529.`
   
   - Family: `Titanium`
   
   - Device: `Ti375C529`
   
   - Timing Model: `C4`

3. Open IP Catalog, select `Processors and Peripherals -> Sapphire High Performance SoC`. Then, click `Next`.

4. On the `IP Configuration` window, select `SLB` tab.
   
   - Enable `I2C controller 0`
   
   - Enable `GPIO controller 0`

5. Click `Generate` to generate the SoC configuration files.

6. Click `Ok` after the soc successfully generated.

## Part 2: Modify bootloader

You are require to modify the bootloader when generating a custom SoC. See [Modify fpga bootloader](../setup/modify_fpga_bootloader.md) document.

## Part 3: Compile the Efinity Project

1. Compile the Efinity project by clicking the `Synthesis` button. After compilation complete, check the output file in `outflow/soc.hex`. `soc.hex` file will be used to program the devkit togather with `fw_jump.bin` and `u-boot.bin`.
   
   > If the timing is not meet then use optimization setting `TIMING_2` or `TIMING_3`. Change it at `FILE -> Edit Project -> Place and Route -> Optimization Level -> TIMING_3`.
