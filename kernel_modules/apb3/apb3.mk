##############################################
#
# APB3 kernel module
#
##############################################

APB3_VERSION = 1.0
APB3_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/kernel_modules/apb3/src
APB3_SITE_METHOD = local

define APB3_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define APB3_INSTALL_TARGET_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules_install
endef

define APB3_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_MODULE_UNLOAD)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
