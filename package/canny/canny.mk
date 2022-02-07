##############################################
#
# canny
#
##############################################

CANNY_VERSION = 1.0
CANNY_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/canny/src
CANNY_SITE_METHOD = local

CANNY_EXTRA_CFLAGS = \
	`$(PKG_CONFIG_HOST_BINARY) --cflags opencv`

CANNY_MAKE_ENV = \
	CFLAGS="$(TARGET_CFLAGS) $(CANNY_EXTRA_CFLAGS)"

define CANNY_BUILD_CMDS
	$(MAKE) CXX=$(TARGET_CXX) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define CANNY_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/canny
	$(INSTALL) -m 0755 -D $(@D)/canny $(TARGET_DIR)/usr/bin/canny
	$(INSTALL) -m 0664 -D $(@D)/image.jpg $(TARGET_DIR)/usr/share/canny/image.jpg
endef

$(eval $(generic-package))
