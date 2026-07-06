OPENAMP_APP_VERSION = 1.0
OPENAMP_APP_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/openamp_app/src
OPENAMP_APP_SITE_METHOD = local

define OPENAMP_APP_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define OPENAMP_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/openamp_app $(TARGET_DIR)/usr/bin/openamp_app
endef

$(eval $(generic-package))
