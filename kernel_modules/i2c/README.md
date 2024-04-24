# I2C Bus Driver

Efinix Sapphire I2C Bus driver support the following features

- master and slave mode

- 7 bit slave address

- speed
  
  - 100 kHz (standard mode)
  
  - 400 kHz (fast mode)
  
  - 1 MHz (fast mode plus)

## Device Tree Binding

The I2C bus driver has compatible string of `efinix,efx-i2c`.

This is an example of generated device tree file in `boards/efinix/common/dts/sapphire.dtsi`. 

- `clock-frequency` 
  
  - Frequency of bus clock in Hz. For example, `100000` is the speed of I2C in standard mode. User can change the I2C speed mode by set it to `400000` Hz or `1000000` Hz respectively.

Optional Property

- efx,i2c-sampling-clock-divider
  
  - Controls the rate at which the I2C controller samples SCL and SDA. By default, it set to 3. FCLK is io_systemClk or io_peripheralClk.
    
    ```
    sampling rate = FCLK / (sampling clock divider + 1)
    ```

Example of device tree binding

```
i2cA: i2c@17000 {
 reg = <0x17000 0x100>;
 compatible = "efinix,efx-i2c";
 interrupts = <8>;
 interrupt-parent = <&plic>;
 clocks = <&apbA_clock 0>;
 clock-frequency = <100000>;
 status = "disabled";
};
```

Example of i2c slave binding for the on Bosch BMP280 temperature, pressure and humidity sensor. Copy the i2c slave binding to `boards/efinix/t120f324/linux/linux.dts`for testing the driver.

```bash
&i2cA {
        #address-cells = <1>;
        #size-cells = <0>;
        status = "okay";
        temperature@76 {
                compatible = "bosch,bmp280";
                reg = <0x76>;
        };
};
```

## Linux Kernel Configuration

Linux I2C subsystem is not enable by default. To enable it for `T120F324` board, edit this file `boards/efinix/t120f324/linux/linux.config`. Add these lines to that file.

```kconfig
CONFIG_I2C=y
CONFIG_I2C_CHARDEV=y
CONFIG_I2C_GPIO=y
CONFIG_I2C_SLAVE=y
CONFIG_I2C_SLAVE_EEPROM=y
```

## Build I2C Bus Driver

1. Enable I2C bus driver in buildroot defconfig. For `T120F324` board the buildroot defconfig is in `configs/efinix_t120f324_defconfig`. You also need to install `i2c-tools` for detecting i2c bus number.
   
   ```
   BR2_PACKAGE_I2C=y
   BR2_PACKAGE_I2C_TOOLS=y
   ```

2. Prepare the build directory and build the I2C bus driver.
   
   ```bash
   . init.sh <board> <soc.h> -r
   make -j$(nproc)
   ```

3. After the build complete, you can flash the Linux image `sd_image.img` into the SD card and boot the Linux.

## Load I2C Bus Driver

After the Linux boot, load the driver using `modprobe` . Once the driver loaded, it will behave as master mode.

### Using modprobe

```bash
modprobe i2c
```

### ## Test I2C Bus Driver

You can use `i2c-tools` such as `i2cdetect` to detect the I2C bus number, I2C slave device address that connect to the I2C bus.

For example to detect I2C bus number and slave address connected to the I2C bus. Here, the I2C bus number is `i2c-0` and I2C slave device detected is `0x76`.

```bash
root@buildroot:~# i2cdetect -l
i2c-0   i2c             EFX I2C (0xf8017000)                    I2C adapter

root@buildroot:~# i2cdetect -y 0
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- 76 -- 
```

### Read the I2C slave device

Uset `i2cget` command to read the data from I2C slave device. For example, to read the device ID of BMP280 using `i2cget`. BMP280 has slave address of `0x76`. `0xd0` is the register address of BMP280 for the device ID. The return value of device ID is `0x58`.

```bash
i2cget -y 0 0x78 0xd0
0x58
```

### Write to I2C slave device

Use `i2cset` command to write to I2C slave device. For example, to reset the BMP280 I2C device. `0xe0` register is the address for reset the BMP280 with `0xb6` as reset command.

```bash
i2cset -y 0 0x76 0xe0 0xb6
```

## I2C Bus Driver as Slave Mode

Efinix I2C bus driver also support slave mode. I2C slave mode will be using interrupt whenever it receive the data. I2C slave mode leverage on `i2c-slave-eeprom` driver backend to behave as eeprom device. Other I2C master (microcontroller) could communicate with Efinix I2C slave.

To enable the I2C slave

```bash
echo slave-24c02 0x1067 > /sys/bus/i2c/devices/i2c-0/new_device
```

I2C master could use `i2cset` and `i2cget` to write to I2C slave. Check the content of the eeprom on slave.

```bash
hexdump -C /sys/bus/i2c/devices/0-1067/slave-eeprom
```

## Unload the I2C Bus Driver

Unloading the I2C bus driver will remove it from the kernel. Thus, the I2C will no longer work after unload.

```bash
rmmod i2c
```
