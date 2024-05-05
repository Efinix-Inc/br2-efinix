##############################################
#
# evsoc_app
#
##############################################

EVSOC_APP_VERSION = 1.0
EVSOC_APP_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/evsoc_app/src
EVSOC_APP_SITE_METHOD = local

EVSOC_APP_EXTRA_CFLAGS = \
	`$(PKG_CONFIG_HOST_BINARY) --cflags opencv`

EVSOC_APP_MAKE_ENV = \
	CFLAGS="$(TARGET_CFLAGS) $(EVSOC_APP_EXTRA_CFLAGS)"
	
define EVSOC_APP_BUILD_CMDS
	$(MAKE) CXX=$(TARGET_CXX) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define EVSOC_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/evsoc_app $(TARGET_DIR)/usr/bin/evsoc_app
endef

$(eval $(generic-package))
