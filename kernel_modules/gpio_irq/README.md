## GPIO Interrupt

There are 2 GPIO interrupt capable pins on Sapphire SoC, which is on gpio pin 0 and 1.
The `gpio_irq` kernel module demonstrate the gpio interrupt on gpio pin 0 which connect to switch 4 (sw4) on Ti375C529 devkit. Whenever user press the sw4 button, and interrupt from the gpio_irq would be triggerd.

### Device Tree
Append this device tree node to the `linux.dtb` file.
```
/ {
    gpio-irq-example {
        compatible = "efinix,gpio-irq";
        interrupt-parent = <&gpio0>;
        interrupts = <0 IRQ_TYPE_EDGE_RISING>;
    };
};
```

### Load the driver
```
modprobe gpio_irq
```
