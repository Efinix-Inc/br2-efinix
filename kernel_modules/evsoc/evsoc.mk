##############################################
#
# APB3 kernel module
#
##############################################

EVSOC_VERSION = 1.0
EVSOC_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/kernel_modules/evsoc/src
EVSOC_SITE_METHOD = local

define EVSOC_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define EVSOC_INSTALL_TARGET_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules_install
endef

define EVSOC_LINUX_CONFIG_FIXUPS
	$(call KCONFIG_ENABLE_OPT,CONFIG_MODULE_UNLOAD)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
