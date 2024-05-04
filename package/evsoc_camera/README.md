## EVSoC Camera

A simple http server which serve camera stream output over the web using mjpeg format.
This application is dedicated for Edge Vision SoC (EVSoC). It require a custom driver support called evsoc.

### Hardware Requirement
1. Ti375C529 Development board
2. Raspberry Pi Camera V3 module
3. HDMI daughter card
4. Ethernet cable

### Hardware Setup
1. Connect the Raspberry Pi Camera V3 module to QSE P2.
2. Connect the HDMI daughter card to QSE P1. 

### How to build?
1. Enable these configuration in the `br2-efinix/config/efinix_ti375c529_defconfig`.

```
BR2_PACKAGE_EVSOC=y
BR2_GCC_ENABLE_OPENMP=y
BR2_PACKAGE_EVSOC_CAMERA=y
```

2. Run `init.sh` script.
```
$ source init.sh ti375c529 /path/to/soc.h -p -r
```

3. Build it
```
$ make -j$(nproc)
```

### How to run?
1. Setup the network on the board.
2. Install the evsoc driver.
```
$ modprobe evsoc
$ evsoc_camera
```

2. Open the web browser then go to `http://<ip address>:8000`. Replace `ip address` with the ip of the device which run this application.

