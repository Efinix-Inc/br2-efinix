# Watchdog Timer

A watchdog timer is a hardware or software timer used to detect and recover from system malfunctions. Its primary purpose is to ensure that a system continues to operate correctly and does not hang indefinitely.

Here’s how it works:

- The watchdog timer is set with a countdown value (timeout).
- The system or application must periodically "kick" or "reset" the watchdog before the timer expires.
- If the watchdog is not reset in time (due to a software crash, deadlock, or hardware fault), the timer expires and triggers a corrective action—usually a system reset or kernel panic.

### **Why is it important?**

- It provides fault tolerance in embedded systems, servers, and critical applications.
- Prevents systems from staying in an unresponsive state indefinitely.
- Common in automotive, industrial control, and IoT devices.

### **Types of Watchdog Timers**

1. **Hardware Watchdog**: Implemented in hardware, independent of the CPU. More reliable because it works even if the OS crashes.
2. **Software Watchdog**: Implemented in the operating system or application. Easier to configure but less robust.

### Kernel Configuration

Enable kernel configuration for watchdog subsystem in `boards/efinix/<board>/linux/linux.config`, where `<board>` are supported boards such as `ti375c529`.

```
CONFIG_WATCHDOG=y
CONFIG_WATCHDOG_SYSFS=y
CONFIG_EFINIX_WATCHDOG=y
CONFIG_WATCHDOG_PRETIMEOUT_GOV=y
CONFIG_WATCHDOG_PRETIMEOUT_GOV_NOOP=y
CONFIG_WATCHDOG_PRETIMEOUT_GOV_PANIC=y
```

### Device Tree Node

Enable device tree node for watchdog timer in `boards/efinix/common/dts/sapphire.dtsi`. Here is an example of device tree node.

```
watchdog0: watchdog@50000 {
    reg = <0x50000 0x100>;
    compatible = "efx,efx-wdt";
    interrupts = <8>;
    interrupt-parent = <&plic>;
    clocks = <&clock>; 
    clock-frequency = <200000000>;
    status = "okay";
};
```

### Watchdog Daemon Package

Enable the Buildroot configuration for watchdog daemon in `configs/efinix_<board>_defconfig`.

```
BR2_PACKAGE_WATCHDOG=y
```

### Configure Watchdog Daemon

The watchdog deamon configuration file in `/etc/watchdog.conf`. Here is a minimal example of configuration.

```
watchdog-device         = /dev/watchdog
watchdog-timeout        = 60
# Interval between tests. Should be a couple of seconds shorter than
# the hardware time-out value.

interval                = 25

# The number of intervals skipped before a log message is written (i.e.
# a multiplier for 'interval' in terms of syslog messages)

logtick        = 1

# Directory for log files (probably best not to change this)

log-dir         = /var/log/watchdog

# Lock the daemon in to memory as a real-time process. This greatly
# decreases the chance that watchdog won't be scheduled before your
# machine is really loaded.

realtime                = yes
priority                = 1
```

### Start Watchdog Daemon

By default, the watchdog daemon does not start automatically during system boot. To activate it, run the daemon in the background using the appropriate command. Note that once the watchdog is running, terminating it will trigger a kernel panic and cause the system to reset after the watchdog reaches its hard timeout.

```bash
watchdog &
```


