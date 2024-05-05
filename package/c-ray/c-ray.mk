##############################################
#
# c-ray
#
##############################################

C_RAY_VERSION = 1.1
C_RAY_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/c-ray/src
C_RAY_SITE_METHOD = local

define C_RAY_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define C_RAY_INSTALL_TARGET_CMDS
	mkdir $(TARGET_DIR)/usr/share/c-ray
	$(INSTALL) -m 0755 -D $(@D)/c-ray-mt $(TARGET_DIR)/usr/bin/c-ray-mt
	$(INSTALL) -m 0755 -D $(@D)/c-ray-f $(TARGET_DIR)/usr/bin/c-ray-f
	$(INSTALL) -m 0755 -D $(@D)/scene $(TARGET_DIR)/usr/share/c-ray/scene
	$(INSTALL) -m 0755 -D $(@D)/scene_1024x768.jpg $(TARGET_DIR)/usr/share/c-ray/scene_1024x768.jpg
	$(INSTALL) -m 0755 -D $(@D)/sphfract $(TARGET_DIR)/usr/share/c-ray/sphfract
	$(INSTALL) -m 0755 -D $(@D)/RUN* $(TARGET_DIR)/usr/share/c-ray

endef

$(eval $(generic-package))
