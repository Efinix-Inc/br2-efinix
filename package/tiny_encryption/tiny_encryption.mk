##############################################
#
# tiny_encryption
#
##############################################

TINY_ENCRYPTION_VERSION = 1.0
TINY_ENCRYPTION_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/tiny_encryption/src
TINY_ENCRYPTION_SITE_METHOD = local

define TINY_ENCRYPTION_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define TINY_ENCRYPTION_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/tiny_encryption $(TARGET_DIR)/usr/bin/tiny_encryption

endef

$(eval $(generic-package))
