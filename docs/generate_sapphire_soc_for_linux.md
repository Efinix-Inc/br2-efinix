# Generate Efinix Sapphire RISC-V SoC for Linux

The default configuration of Sapphire SoC on IP Manager would not able to boot Linux. In this tutorial we are going to generate a custom Sapphire SoC which can boot Linux on Trion T120F324 development board.

These steps also applicable for Ti60F225 and Ti180J484 with different pinout assignment for SPI 1. Please refer to table below on pinout assignment.

## Prerequsite

- Efinity Software 2023.2 or later

- Efinity RISCV IDE 2023.2 or later

## Install Efinity Software

Follow the official [document](https://www.efinixinc.com/support/docsdl.php?s=ef&pn=UG-EFN-INSTALL) for installing Efinity software.

## Part 1a: Generate Custom Sapphire SoC Configuration

1. Open Efinity software.

2. Create a new project. Select `File -> Create Project`. On `Project Editor` window, put the name as `soc` and click `OK`.

3. Click `IP Manager` and select `Installed IP -> Efinix -> Processors and Peripherals -> Sapphire SoC`. Then click `Next>>`.

4. On `IP Configuration` window, give the `Module Name` as `soc1`.

5. Under `SOC` tab, 
   
   - select `Core Number` to 4
     
     - the core number is configurable up to 4
   
   - set `Frequency` to 50Mhz
     
     - the frequency is configurable but you need to make sure the design close the timing constraint
   
   - enable `Peripheral Clock Frequency` and set it to 50Mhz
     
     - it is recommended to turn on when the system frequency over 100Mhz
     
     - the recommended operating frequency for SPI is between 50Mhz to 100Mhz
   
   - enable `Cache` (compulsory)
   
   - enable `Linux Memory Management Unit` (compulsory)
   
   - enable `Floating-point unit` (optional)
   
   - enable `Compressed Extension` (optional)

6. Under `Cache/Memory` tab,
   
   - For Ti180J484, click `Enable AXI4 full-duplex interface` and set `External Memory Data Width` to `512`.
   
   - Other than Ti180J484, **disabled** `Enable AXI4 full-duplex interface`and set`External Memory Data Width`to`128`.
   
   - Set the `External Memory Address Size` as follows
     
     | Devkit    | External Memory Address Size |
     | --------- | ---------------------------- |
     | T120F324  | 256 MB                       |
     | Ti60F225  | 32 MB                        |
     | Ti180J484 | 256 MB                       |
   
   - At On-Chip RAM Size. set to `8KB`.

7. Under `Debug` tab,
   
   - enable `RISC-V standard debug`
   
   - set `Hardware Breakpoint` to 2

8. Under `SPI` tab, enable SPI 1 for the SD card.

9. Click `Generate` to generate the soc.

10. Click `Ok` after the soc successfully generated.

> `Note:` Peripheral Clock Frequency should be set within the supported frequency of the connected peripheral.

## Part 1b: Assign Pinout for SPI 1

Table below shows the best known configuration for assigning the pinout for SPI 1. The SPI 1 is used for SD card. Please note that `system_uart_0_io_*` also need to reassign for `T120F324`. You can use other pinout by referring to the devkit user guide and schematics at [Efinix support page](https://www.efinixinc.com/support/docs.php).

| Board     | Instance Name                                                                                                                                                  | Pin                                                                                              | Mode                                                            | I/O standard | Clock Signal                      |
| --------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------ | --------------------------------------------------------------- | ------------ | --------------------------------- |
| T120F324  | system_spi_1_io_data_0<br/>system_spi_1_io_data_1<br/>system_spi_1_io_sclk_write<br/>system_spi_1_io_ss<br/><br/>system_uart_0_io_rxd<br/>system_uart_0_io_txd | GPIOT_RXP21<br/>GPIOT_RXP22<br/>GPIOT_RXP23<br/>GPIOT_RXP20<br/><br/>GPIOT_RXP07<br/>GPIOT_RXN07 | inout<br/>inout<br/>output<br/>output<br/><br/>input<br/>output | 3.3 V LVCMOS | *io_peripheralClk or io_systemClk |
| Ti60F225  | system_spi_1_io_data_0<br/>system_spi_1_io_data_1<br/>system_spi_1_io_sclk_write<br/>system_spi_1_io_ss                                                        | GPIOR_16<br/>GPIOR_18<br/>GPIOR_15<br/>GPIOR_13                                                  | inout<br/>inout<br/>output<br/>output                           | 3.3 V LVCMOS | *io_peripheralClk or io_systemClk |
| Ti180J484 | system_spi_1_io_data_0<br/>system_spi_1_io_data_1<br/>system_spi_1_io_sclk_write<br/>system_spi_1_io_ss                                                        | GPIOR_60<br/>GPIOR_61<br/>GPIOR_62<br/>GPIOR_59                                                  | inout<br/>inout<br/>output<br/>output                           | 1.8V LVCMOS  | *io_peripheralClk or io_systemClk |

> `Note:` * if peripheral clock is enable then use `io_peripheralClk` as clock signal. Else use `io_systemClk`.

1. Open the generated soc project. Select `File -> Open Project` then browse to `$EFINITY_HOME/project/soc/ip/soc1/T120F324_devkit/soc.xml` then click `Open`.

2. The SPI 1 will be used by the micro SD card. We are going to create 4 blocks for SPI 1 and assign the pin out of the SPI 1. The 4 blocks as follows:
   
   ```
   system_spi_1_io_data_0
   system_spi_1_io_data_0
   system_spi_1_io_sclk_write
   system_spi_1_io_ss
   ```
   
   Open `Efinity Interface Designer`. On the Interface Designer, click `GPIO(16)` then click `Create Block` icon at the left panel.
   
   a) Create block for `system_spi_1_io_data_0` and `system_spi_1_io_data_1`.
   
   - At the `Block Editor`, set `Instance Name` as `system_spi_1_io_data_0` and hit `Enter`.
   
   - set `Mode` to `inout`.
   
   - at `Input`,
     
     - set `Pin Name` as `system_spi_1_io_data_0_read` and hit `Enter`.
     
     - select `Register` option as `register`.
     
     - set `Clock Pin Name` as `io_peripheralClk` and hit `Enter`.

> You may verify if the setting has been successfully set to the interface by checking on the `Block Summary`.If the `io_peripheralClk` are not shown in the Block Summary after you hit `Enter`, you may want to remove the `io_peripheralClk` setting first, select other block at the GPIO(n) tab. Go back to the `system_spi_1_io_data_0` block and retry the setting of `io_peripheralClk`. 

- at `Output`,
  
  - set `Pin Name` as `system_spi_1_io_data_0_write` and hit `Enter`.
  
  - select `Register` option as `register`.
  
  - set `Clock Pin Name` as `io_peripheralClk` and hit `Enter`.

- at `Output Enable`,
  
  - set `Pin Name` as `system_spi_1_io_data_0_writeEnable` and hit `Enter`.
  
  - select `Register` option as `register`.

- repeat step 2a for `system_spi_1_io_data_1`.
  
  b) Create a new block for `system_spi_1_io_sclk_write` and `system_spi_1_io_ss`.

- set `Instance Name` as `system_spi_1_io_sclk_write` and hit `Enter`.

- set `Mode` as `output`.

- at `Output`,
  
  - set `Pin Name` as `system_spi_1_io_sclk_write.
  
  - select `Register` option as `register`.

- at `Output Clock`,
  
  - set `Pin Name` as `io_peripheralClk` and hit `Enter`.

- repeat step 2b for `system_spi_1_io_ss`.
3. Next step is to assign the pinout for the newly created SPI 1 block. On `Efinity Interface Designer` click `Show/Hide GPIO Resource Assigner` icon at the top of `create Block` icon to open `Resouces Assigner`.
   
   In the `Resource Assigner`, assign the `Resource` of `system_spi_1_io_data_0` as `GPIOT_RXP21`. See the table above for the rest of the block. Make sure you reassign the `system_uart_0_io_rxd` and `system_uart_0_io_txd`.

4. Modify the IO voltage bank for the `SPI1`. On the `Efinity Interface Designer` click `Design: T120F324` -> `Device Setting` -> `I/O Banks`. At the `Block Editor` select `2F` for the `I/O Bank` and set it to `3.3V` at `I/O Voltage`.
   
   You may check the `I/O Bank` for each pin at the `Resources Assigner`.

5. Click `Generate Efinity Constrain Files` icon.

## Part 1c: Modify Top Level Verilog Code

1. Open the `top_soc.v` file at Project tab on the left panel of Efinity software. The file locate in `soc -> Design -> top_soc.v`.

2. Modify `module top_soc ()` by adding these lines between `...`
   
   ```verilog
   module top_soc(
   ...
   output        system_spi_1_io_sclk_write,
   output        system_spi_1_io_data_0_writeEnable,
   input         system_spi_1_io_data_0_read,
   output        system_spi_1_io_data_0_write,
   output        system_spi_1_io_data_1_writeEnable,
   input         system_spi_1_io_data_1_read,
   output        system_spi_1_io_data_1_write,
   output        system_spi_1_io_ss,
   ...
   )
   ```
   
   > The newly added ports must be placed in between other ports. Placing the new ports after the spi_0 ports are preferred. Appending these codes to the last line of the port instantiation will cause some errors due to the comma location. 

3. Modify `soc_inst()` by adding the pin name in the parenthesis `()` and save.
   
   ```verilog
   soc1 soc_inst(
   ...
   .system_spi_1_io_sclk_write(system_spi_1_io_sclk_write),
   .system_spi_1_io_data_0_writeEnable(system_spi_1_io_data_0_writeEnable),
   .system_spi_1_io_data_0_read(system_spi_1_io_data_0_read),
   .system_spi_1_io_data_0_write(system_spi_1_io_data_0_write),
   .system_spi_1_io_data_1_writeEnable(system_spi_1_io_data_1_writeEnable),
   .system_spi_1_io_data_1_read(system_spi_1_io_data_1_read),
   .system_spi_1_io_data_1_write(system_spi_1_io_data_1_write),
   .system_spi_1_io_data_2_writeEnable(),
   .system_spi_1_io_data_2_read(),
   .system_spi_1_io_data_2_write(),
   .system_spi_1_io_data_3_writeEnable(),
   .system_spi_1_io_data_3_read(),
   .system_spi_1_io_data_3_write(),
   .system_spi_1_io_ss(system_spi_1_io_ss),
   ...
   )
   ```

## Part 2: Modify Bootloader

You are require to modify the bootloader when generating a custom SoC. See [Modify fpga bootloader](modify_fpga_bootloader.md) document.

## Part 3: Compile the Efinity Project

1. Compile the Efinity project by clicking the `Synthesis` button. After compilation complete, check the output file in `outflow/soc.hex`. `soc.hex` file will be used to program the devkit togather with `fw_jump.bin` and `u-boot.bin`.
   
   > If the timing is not meet then use optimization setting `TIMING_2` or `TIMING_3`. Change it at `FILE -> Edit Project -> Place and Route -> Optimization Level -> TIMING_3`.
