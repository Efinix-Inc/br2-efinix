##############################################
#
# gpiod
#
##############################################

GPIOD_VERSION = 1.0
GPIOD_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/gpiod/src
GPIOD_SITE_METHOD = local

define GPIOD_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define GPIOD_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/gpiod $(TARGET_DIR)/usr/bin/gpiod
endef

$(eval $(generic-package))
