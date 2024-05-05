## EVSOC

A custom kernel driver for camera and HDMI display which use EVSoC design.
The custom driver support Raspberry PiCam module 2 and Raspberry PiCam module 3.
The FPGA design support for Ti375C529 is located in this path.
`br2-efinix/boards/efinix/ti375c529/hardware/unified_hw`.

### Hardware Requirement
1. Ti375C529 Development board
2. Raspberry Pi Camera V3 module
3. HDMI daughter card

### Hardware Setup
1. Connect the Raspberry Pi Camera V3 module to QSE P2.
2. Connect the HDMI daughter card to QSE P1.

### How to build?
1. Enable these configuration in the `br2-efinix/config/efinix_ti375c529_defconfig`.

```
BR2_PACKAGE_EVSOC=y
```

2. Run `init.sh` script.
```
$ source init.sh ti375c529 /path/to/soc.h -p -r
```

3. Build it
```
$ make -j$(nproc)
```

### How to load the driver?
1. Boot up the kernel then use modprobe to load the driver.

```
modprobe evsoc
```
