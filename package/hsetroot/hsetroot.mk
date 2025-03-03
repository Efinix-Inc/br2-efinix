################################################################################
#
# hsetroot
#
################################################################################

HSETROOT_VERSION = a402d5323f45c058810c0d66d61d278d101d3610
HSETROOT_DESCRIPTION = Yet another wallpaper application
HSETROOT_SITE = $(call github,himdel,hsetroot,$(HSETROOT_VERSION))
HSETROOT_LICENSE = MIT
HSETROOT_LICENSE_FILES = LICENSE

define HSETROOT_BUILD_CMDS
	PKG_CONFIG_PATH=$(STAGING_DIR)/usr/lib/pkgconfig:$(STAGING_DIR)/usr/share/pkgconfig \
	$(MAKE) CC=$(TARGET_CC) PKG_CFG=$(PKG_CONFIG_HOST_BINARY) -C $(@D)
endef

define HSETROOT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hsetroot $(TARGET_DIR)/usr/bin/hsetroot
	$(INSTALL) -D -m 0755 $(@D)/hsr-outputs $(TARGET_DIR)/usr/bin/hsr-outputs
endef

$(eval $(generic-package))
