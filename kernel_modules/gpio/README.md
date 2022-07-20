# GPIO Consumer Driver

GPIO consumer driver for Efinix RISC-V Sapphire SoC used to control the GPIO pin. The default configuration of Efinix RISC-V Sapphire SoC already enable 4 GPIO pins on the GPIO port A. 3 pins for LEDs and 1 pin for the switch button.

This is an example of generated device tree file in `boards/efinix/common/dts/sapphire.dtsi`.

```
gpioA: gpio@16000 {
    reg = <0x16000 0x100>;
    compatible = "spinal-lib,gpio-1.0";
    interrupts = <13>;
    interrupt-parent = <&plic>;
    ngpio = <4>;
    gpio-controller;
    #gpio-cells = <2>;
    #interrupt-cells = <2>;
    clocks = <&apbA_clock 0>;
    status = "disabled";
};
```

Please note that `ngpio` is the number of gpio pin gets enable. You may change this number according to your design.

## Device Tree Binding

The GPIO consumer driver has compatible string of `efinix,efx-gpio`. The important property of `gpio driver` is `efx-gpios`. It received 3 cells to describe the usage of the gpio pin with the format

```
efx-gpios = <&gpio_controller pin_number gpio_active_state>;
```

For example,

```
efx-gpios = <&gpioA 0 GPIO_ACTIVE_LOW>;
```

- `&gpioA` is the phandle of `gpio_controller` port controller.

- `0` is the the pin number of the `gpio_controller` port controller.

- `GPIO_ACTIVE_LOW` is the state of the gpio to active. The pin `0` on `gpioA` is active when the signal is `low`.

Example of gpio binding for the on board LEDs. Copy the gpio binding to `boards/efinix/t120f324/linux/linux.dts`for testing the driver.

```
efx_gpio_devs {
        compatible = "efinix,efx-gpio";
        sw {
            label = "SW";
            efx-gpios = <&gpioA 0 GPIO_ACTIVE_LOW>;
        };
        led1 {
            label = "LED1";
            efx-gpios = <&gpioA 1 GPIO_ACTIVE_HIGH>;
        };
        led2 {
            label = "LED2";
            efx-gpios = <&gpioA 2 GPIO_ACTIVE_HIGH>;
        };
        led3 {
            label = "LED3";
            efx-gpios = <&gpioA 3 GPIO_ACTIVE_HIGH>;
        };
    };
```

## Build GPIO Driver

1. Enable GPIO driver in buildroot defconfig. For `T120F324` board the buildroot defconfig is in `configs/efinix_t120f324_defconfig`. You may also can enable `gpio_app` for testing the GPIO driver.
   
   ```Kconfig
   BR2_PACKAGE_GPIO=y
   BR2_PACKAGE_GPIO_APP=y
   ```

2. Prepare the build directory and build the GPIO driver and GPIO application.
   
   ```bash
   mkdir build && cd build
   make O=$PWD \
   BR2_EXTERNAL=../br2-efinix \
   -C ../buildroot \
   efinix_t120f324_defconfig
   make -j$(nproc)
   ```

3. After the build complete, you can flash the Linux image `sd_image.img` into the SD card and boot the Linux.

## Load GPIO Driver

After the Linux boot, load the driver using `modprobe` or `insmod`.

### Using modprobe

```bash
modprobe gpio
```

### Using insmod

```bash
kernel_version=$(uname -r)
cd /lib/modules/${kernel_version}/extra 
insmod gpio.ko
```

## Test GPIO Driver

Run the GPO application and observed the LEDs on board blinking. Press `ctrl+c` to stop the application.

```bash
gpio_app
```

You also can turn ON/OFF the LED manually. For example to turn ON/OFF LED1. Set the direction to `out`.

```bash
echo out > /sys/class/efx_gpios/LED1/direction
```

Turn ON LED1

```bash
echo 0 > /sys/class/efx_gpios/LED1/value
```

Turn OFF LED1

```bash
echo 1 > /sys/class/efx_gpios/LED1/value
```

## Unload the GPIO driver

Unloading the GPIO driver will remove it from the kernel. Thus, the GPIO will no longer work after unload.

```bash
rmmod gpio
```
