##############################################
#
# gpio_app
#
##############################################

GPIO_APP_VERSION = 1.0
GPIO_APP_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/gpio_app/src
GPIO_APP_SITE_METHOD = local

define GPIO_APP_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define GPIO_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/gpio_app $(TARGET_DIR)/usr/bin/gpio_app
endef

$(eval $(generic-package))
