##############################################
#
# cv2_detection
#
##############################################

CV2_DETECTION_VERSION = 1.0
CV2_DETECTION_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/cv2_detection/src
CV2_DETECTION_SITE_METHOD = local

CV2_DETECTION_EXTRA_CFLAGS = \
	`$(PKG_CONFIG_HOST_BINARY) --cflags opencv`

CV2_DETECTION_MAKE_ENV = \
	CFLAGS="$(TARGET_CFLAGS) $(CV2_DETECTION_EXTRA_CFLAGS)"

define CV2_DETECTION_BUILD_CMDS
	$(MAKE) CXX=$(TARGET_CXX) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define CV2_DETECTION_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/cv2_detection
	$(INSTALL) -m 0755 -D $(@D)/cv2_detection $(TARGET_DIR)/usr/bin/cv2_detection
	$(INSTALL) -m 0664 -D $(@D)/cat_dog.jpg $(TARGET_DIR)/usr/share/cv2_detection/cat_dog.jpg
	$(INSTALL) -m 0664 -D $(@D)/MobileNetSSD_deploy.caffemodel $(TARGET_DIR)/usr/share/cv2_detection/MobileNetSSD_deploy.caffemodel
	$(INSTALL) -m 0664 -D $(@D)/MobileNetSSD_deploy.prototxt.txt $(TARGET_DIR)/usr/share/cv2_detection/MobileNetSSD_deploy.prototxt.txt
endef

$(eval $(generic-package))
