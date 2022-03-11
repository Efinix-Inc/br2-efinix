##############################################
#
# GPIO
#
##############################################

GPIO_VERSION = 1.0
GPIO_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/kernel_modules/gpio/src
GPIO_SITE_METHOD = local

define GPIO_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define GPIO_INSTALL_TARGET_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules_install
endef

define GPIO_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_MODULE_UNLOAD)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
