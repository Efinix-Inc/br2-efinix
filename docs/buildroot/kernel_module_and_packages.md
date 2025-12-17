Kernel Module and Packages
==========================

Buildroot external tree (BR2-external) support building packages and out-of-tree kernel module. For more info on Buildroot please see the [Buildroot official document](https://buildroot.org/downloads/manual/manual.html#_getting_started).

Adding your own Kernel module
-----------------------------

Kernel module can be placed in the BR2-external tree or hosting on another repository.
There is a specific file structure to follow when using BR2-external tree.
Below is an example of file structre that storing foo kernel module in the BR2-external.

```
<BR2-external>
├── Config.in
├── external.desc
├── external.mk
├── configs
│   └── target_buildroot_defconfig
└── kernel_modules
    └── foo
        ├── foo.mk
        ├── Config.in
        └── src
            ├── foo.c
            └── Kbuild
```

### Config.in

This file is should be created as it is sourcing the `kernel_modules` and `packages` directory. All of kernel modules and packages need to be specified in this file so that Buildroot know where to source and build. For example to include `foo` kernel module in `Config.in` as follows

```bash
source "$BR2_EXTERNAL_PATH/kernel_modules/foo/Config.in"
```

### external.mk

This file also is should be created to store the path of makefile `(*.mk)` for `kernel module` and `packages`. For example to include kernel module makefile in `external.mk`,

```bash
include $(sort $(wildcard $(BR2_EXTERNAL_PATH)/kernel_modules/*/*.mk))
```

### kernel_modules/foo

Folder name of kernel module.

### kernel_modules/foo/foo.mk

Makefile of `foo` kernel module.

### kernel_modules/foo/Config.in

The BR2 configuration to select the kernel module. It is same as Linux Kconfig format.   Example of content of this file.

```Kconfig
config BR2_PACKAGE_FOO
    bool "foo module"
    depends on BR2_LINUX_KERNEL
    default n
    help
      foo module
```

### kernel_modules/foo/src

This folder contain the source code of kernel module. This is an optional folder and only needed if kernel module is store in BR2-external tree.

### kernel_modules/foo/src/Kbuild

This file same as Makefile of Linux. It specify the foo to build as kernel module.   For example,

```makefile
obj-m += foo.o
```

## How To build foo kernel module?

### Enable the module

Make sure the config of kernel module is enable in `configs/target_buildroot_defconfig` file. For example to enable building `foo` kernel module. in the `configs/target_buildroot_defconfig`,

```Kconfig
BR2_PACKAGE_FOO=y
```

### Build the module

```bash
mkdir build && cd build
make O=$PWD \
BR2_EXTERNAL=<path/to/br2-external> \
-C <path/to/buildroot> \
<target_buildroot_defconfig>
make foo
```

The kernel module will be copy to `build/target/lib/modules/<kernel_version>/extra/foo.ko`.

### Load the kernel module on running Linux

```bash
cd /lib/modules/<kernel_version>/extra 
insmod foo.ko
```

In this repository, there are some examples of kernel modules which can guide you on how to enable your own kernel module.

- [helloworld](../../kernel_modules/helloworld/README.md)

Adding your own custom package
------------------------------

Buildroot also support adding custom package on BR2-external tree. Below is an example of package file structure that need to follow on BR2-external. Please note that the file structure same as kernel module but the only different is no need to add `Kbuild` file. The `bar` package source file is store in the BR2-external tree in this example. you also can host the source file into a repository.

```
<BR2-external>
├── Config.in
├── external.desc
├── external.mk
└── package
    └── bar
        ├── bar.mk
        ├── Config.in
        └── src
            ├── bar.c
            └── Makefile
```
