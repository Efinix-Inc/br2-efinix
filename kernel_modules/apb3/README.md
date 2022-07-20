# APB3 Driver

APB3 driver is a driver for pseudorandom number generator (PRNG) that is hanging on APB3 bus. This driver allow user to read the value of PRNG, start and stop the random number generation.

## Build APB3 Driver

1. Enable the driver in Buildroot defconfig. For `T120F324` board the buildroot defconfig is in `configs/efinix_t120f324_defconfig`. You also may enable the APB3 demo application.
   
   ```
   BR2_PACKAGE_APB3=y
   BR2_PACKAGE_APB3APP=y
   ```

2. Prepare the build directory and build it.
   
   ```bash
   mkdir build && cd build
   make O=$PWD \
   BR2_EXTERNAL=../br2-efinix \
   -C ../buildroot \
   efinix_t120f324_defconfig
   make -j$(nproc)
   ```

## Load APB3 driver

```bash
modprobe apb3
```

## Unload APB3 driver

```bash
rmmod apb3
```

## Run APB3 Demo Application

The apb3 demo application demostrate the functionality of the apb3 driver. It read the value of the of the PRNG every seconds for three times. Then, it stop the PRNG and read again to show that the PRNG has stopped. After that, it start the PRNG and read the value again.

```bash
apb3app
```
