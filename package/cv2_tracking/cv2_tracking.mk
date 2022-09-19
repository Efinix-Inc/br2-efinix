##############################################
#
# cv2_tracking
#
##############################################

CV2_TRACKING_VERSION = 1.0
CV2_TRACKING_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/cv2_tracking/src
CV2_TRACKING_SITE_METHOD = local

CV2_TRACKING_EXTRA_CFLAGS = \
	`$(PKG_CONFIG_HOST_BINARY) --cflags opencv`

CV2_TRACKING_MAKE_ENV = \
	CFLAGS="$(TARGET_CFLAGS) $(CV2_TRACKING_EXTRA_CFLAGS)"

define CV2_TRACKING_BUILD_CMDS
	$(MAKE) CXX=$(TARGET_CXX) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define CV2_TRACKING_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/cv2_tracking
	$(INSTALL) -m 0755 -D $(@D)/cv2_tracking $(TARGET_DIR)/usr/bin/cv2_tracking
	$(INSTALL) -m 0664 -D $(@D)/sample_video_320p_6fps_5sec.mp4 $(TARGET_DIR)/usr/share/cv2_tracking/sample_video_320p_6fps_5sec.mp4
endef

$(eval $(generic-package))
