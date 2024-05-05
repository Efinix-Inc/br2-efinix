##############################################
#
# raystone
#
##############################################

RAYSTONE_VERSION = 1.0
RAYSTONE_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/raystone/src
RAYSTONE_SITE_METHOD = local

define RAYSTONE_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define RAYSTONE_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/raystone $(TARGET_DIR)/usr/bin/raystone
endef

$(eval $(generic-package))
