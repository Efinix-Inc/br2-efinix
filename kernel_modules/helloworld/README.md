# Helloworld Kernel Module

Hello world kernel module is a simple example of kernel module to help developer to get started with the Buildroot framework. This module just print "hello world" when it loaded and print "Bye" when it get removed.

## Build Helloworld kernel module

1. Enable the Buildroot configuration in. For `T120F324` board the buildroot defconfig is in `configs/efinix_t120f324_defconfig`.
   
   ```
   BR2_PACKAGE_HELLOWORLD=y
   ```

2. Prepare build directory and start build
   
   ```bash
   mkdir build && cd build
   make O=$PWD \
   BR2_EXTERNAL=../br2-efinix \
   -C ../buildroot \
   efinix_t120f324_defconfig
   make -j$(nproc)
   ```

## Load Helloworld kernel module

```bash
modprobe helloworld
```

## Unload Helloworld kernel module

```bash
rmmod helloworld
```
