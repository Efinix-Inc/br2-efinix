##############################################
#
# apb3app
#
##############################################

APB3APP_VERSION = 1.0
APB3APP_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/apb3app/src
APB3APP_SITE_METHOD = local

define APB3APP_BUILD_CMDS
	$(MAKE) CXX=$(TARGET_CXX) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define APB3APP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/apb3app $(TARGET_DIR)/usr/bin/apb3app
endef

$(eval $(generic-package))
