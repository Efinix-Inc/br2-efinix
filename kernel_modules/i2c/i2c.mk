##############################################
#
# I2C
#
##############################################

I2C_VERSION = 1.0
I2C_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/kernel_modules/i2c/src
I2C_SITE_METHOD = local

define I2C_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define I2C_INSTALL_TARGET_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules_install
endef

define I2C_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_MODULE_UNLOAD)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
