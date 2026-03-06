################################################################################
#
# Buildroot package for Infineon backports kernel modules
#
################################################################################

IFX_WIFI_SITE = https://github.com/Infineon/ifx-backports.git
IFX_WIFI_SITE_METHOD = git
IFX_WIFI_COMMIT = 3f503c3a33dd2e45b94a3f93b5ddd950b4268e71

IFX_WIFI_VERSION = $(IFX_WIFI_COMMIT)
IFX_WIFI_LICENSE = GPL-2.0
IFX_WIFI_LICENSE_FILES = LICENSE
IFX_WIFI_DEPENDENCIES = ifx-wifi-fw ifx-wifi-nvram

# Declare it's a custom source package (git)
$(eval $(generic-package))

KERNEL_RELEASE := $(shell make -s -C $(LINUX_DIR) kernelrelease)

# Define build steps
define IFX_WIFI_BUILD_CMDS
    # export MY_KERNEL=$(LINUX_DIR)
    # export CROSS_COMPILE=$(TARGET_CROSS)

    # Fix permissions for kconfig helper and make
    chmod +x $(@D)/v6.1.145-backports/kconf/lxdialog/check-lxdialog.sh
    chmod +x $(@D)/v6.1.145-backports/scripts/make

    # Copy brcmfmac defconfig into v6.1.145-backports folder
    cp $(@D)/v6.1.145-backports/defconfigs/brcmfmac \
       $(@D)/v6.1.145-backports/.config

    # Merge with current kernel config inside v6.1.145-backports
    $(MAKE) -C $(@D)/v6.1.145-backports \
        KLIB=$(LINUX_DIR) KLIB_BUILD=$(LINUX_DIR) \
        ARCH=riscv CROSS_COMPILE=$(TARGET_CROSS) defconfig-brcmfmac

    # Build modules inside v6.1.145-backports
    $(MAKE) -C $(@D)/v6.1.145-backports \
        KLIB=$(LINUX_DIR) KLIB_BUILD=$(LINUX_DIR) \
        ARCH=riscv CROSS_COMPILE=$(TARGET_CROSS) modules
endef

define IFX_WIFI_INSTALL_TARGET_CMDS

    # Install built modules to target
    mkdir -p $(TARGET_DIR)/lib/modules/$(KERNEL_RELEASE)/extra/ifx/

    # Copy all built .ko files from the backports build
    @echo "=== Installing IFX WiFi kernel modules to target ==="
    cp $(@D)/v6.1.145-backports/compat/compat.ko \
       $(TARGET_DIR)/lib/modules/$(KERNEL_RELEASE)/extra/ifx/
    cp $(@D)/v6.1.145-backports/drivers/net/wireless/broadcom/brcm80211/brcmfmac/brcmfmac.ko \
       $(TARGET_DIR)/lib/modules/$(KERNEL_RELEASE)/extra/ifx/
    cp $(@D)/v6.1.145-backports/drivers/net/wireless/broadcom/brcm80211/brcmutil/brcmutil.ko \
       $(TARGET_DIR)/lib/modules/$(KERNEL_RELEASE)/extra/ifx/
    cp $(@D)/v6.1.145-backports/net/wireless/cfg80211.ko \
       $(TARGET_DIR)/lib/modules/$(KERNEL_RELEASE)/extra/ifx/
    @echo "=== Installing IFX WiFi kernel modules to target: DONE ==="
endef

