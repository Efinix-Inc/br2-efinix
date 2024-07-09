##############################################
#
# evsoc_camera
#
##############################################

EVSOC_CAMERA_VERSION = 2.0
EVSOC_CAMERA_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/evsoc_camera/src
EVSOC_CAMERA_SITE_METHOD = local

EVSOC_CAMERA_EXTRA_CFLAGS = \
        `$(PKG_CONFIG_HOST_BINARY) --cflags opencv`

EVSOC_CAMERA_MAKE_ENV = \
        CFLAGS="$(TARGET_CFLAGS) $(EVSOC_CAMERA_EXTRA_CFLAGS)"

define EVSOC_CAMERA_BUILD_CMDS
        $(MAKE) CXX=$(TARGET_CXX) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define EVSOC_CAMERA_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/evsoc_camera
        $(INSTALL) -m 0755 -D $(@D)/evsoc_camera $(TARGET_DIR)/usr/bin/evsoc_camera
	$(INSTALL) -m 0644 -D $(@D)/web_root/index.html $(TARGET_DIR)/usr/share/evsoc_camera/web_root/index.html
endef

$(eval $(generic-package))
