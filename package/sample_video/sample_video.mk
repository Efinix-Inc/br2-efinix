define SAMPLE_VIDEO_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/sample_videos
	$(INSTALL) -m 0664 -D $(BR2_EXTERNAL_EFINIX_PATH)/package/sample_video/src/* $(TARGET_DIR)/usr/share/sample_videos/
endef

$(eval $(generic-package))
