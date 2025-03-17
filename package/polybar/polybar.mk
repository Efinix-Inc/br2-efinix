################################################################################
#
# polybar
#
################################################################################

POLYBAR_VERSION = 3.5.7
POLYBAR_DESCRIPTION = A fast and easy-to-use status bar
POLYBAR_SITE = $(call github,polybar,polybar,$(POLYBAR_VERSION))
POLYBAR_LICENSE = MIT
POLYBAR_LICENSE_FILES = LICENSE
POLYBAR_DEPENDENCIES = iw xcb-util-wm

XPP_VERSION = 044e69d05db7f89339bda1ccd1efe0263b01c8f6
XPP_SITE = https://github.com/polybar/xpp/archive
XPP_FILE = $(XPP_VERSION).zip

I3IPCPP_VERSION = 86ddf7102c6903ae0cc543071e2d375403fc0727
I3IPCPP_SITE = https://github.com/polybar/i3ipcpp/archive
I3IPCPP_FILE = $(I3IPCPP_VERSION).zip

POLYBAR_EXTRA_DOWNLOADS = \
	$(XPP_SITE)/$(XPP_FILE) \
	$(I3IPCPP_SITE)/$(I3IPCPP_FILE)

define POLYBAR_EXTRACT_SUBMODULE_XPP
	rm -rf $(@D)/lib/xpp && \
	$(UNZIP) -q $(POLYBAR_DL_DIR)/$(XPP_FILE) -d $(@D)/lib && \
	mv $(@D)/lib/xpp-$(XPP_VERSION) $(@D)/lib/xpp
endef

define POLYBAR_EXTRACT_SUBMODULE_I3IPCPP
	rm -rf $(@D)/lib/i3ipcpp  && \
	$(UNZIP) -q $(POLYBAR_DL_DIR)/$(I3IPCPP_FILE) -d $(@D)/lib && \
	mv $(@D)/lib/i3ipcpp-$(I3IPCPP_VERSION) $(@D)/lib/i3ipcpp
endef

POLYBAR_POST_EXTRACT_HOOKS += \
	POLYBAR_EXTRACT_SUBMODULE_XPP \
	POLYBAR_EXTRACT_SUBMODULE_I3IPCPP

ifeq (BR2_PACKAGE_IW,y)
POLYBAR_CONF_OPTS += -DENABLE_NETWORK=ON
endif

$(eval $(cmake-package))
