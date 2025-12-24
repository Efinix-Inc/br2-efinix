##############################################
#
# gpio_irq
#
##############################################

GPIO_IRQ_VERSION = 1.0
GPIO_IRQ_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/kernel_modules/gpio_irq/src
GPIO_IRQ_SITE_METHOD = local

define GPIO_IRQ_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define GPIO_IRQ_INSTALL_TARGET_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules_install
endef

define GPIO_IRQ_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_MODULE_UNLOAD)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
