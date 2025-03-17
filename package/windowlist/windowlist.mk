################################################################################
#
# windowlist
#
################################################################################
WINDOWLIST_VERSION = 27530c98f0bb2cde910515f0f135eb427808065e
WINDOWLIST_DESCRIPTION = Polybar custom module - List all open windows, click to switch focus or minimize
WINDOWLIST_SITE = $(call github,tuurep,windowlist,$(WINDOWLIST_VERSION))
WINDOWLIST_DEPENDENCIES = xapp_xprop wmctrl
WINDOWLIST_LICENSE = MIT
WINDOWLIST_LICENSE_FILES = LICENSE

WINDOWLIST_EXTRA_CFLAGS = -Wall `$(PKG_CONFIG_HOST_BINARY) --cflags x11`
WINDOWLIST_EXTRA_LDFLAGS = `$(PKG_CONFIG_HOST_BINARY) --libs x11`

define WINDOWLIST_BUILD_CMDS
	$(MAKE) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS) $(WINDOWLIST_EXTRA_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS) $(WINDOWLIST_EXTRA_LDFLAGS)"  -C $(@D)
endef

define WINDOWLIST_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/root/.config/windowlist
	$(INSTALL) -D -m 0755 $(@D)/windowlist $(TARGET_DIR)/root/.config/windowlist/windowlist
	$(INSTALL) -D -m 0755 $(@D)/click-actions/close $(TARGET_DIR)/root/.config/windowlist/click-actions/close
	$(INSTALL) -D -m 0755 $(@D)/click-actions/minimize $(TARGET_DIR)/root/.config/windowlist/click-actions/minimize
	$(INSTALL) -D -m 0755 $(@D)/click-actions/raise $(TARGET_DIR)/root/.config/windowlist/click-actions/raise
endef

$(eval $(generic-package))
