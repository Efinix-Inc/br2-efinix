################################################################################
#
# picom
#
################################################################################

PICOM_VERSION = v12.5
PICOM_DESCRIPTION = A lightweight compositor for X11 with animation support
PICOM_SITE = https://github.com/yshui/picom/archive/$(PICOM_VERSION)/picom-$(PICOM_VERSION).tar.gz
PICOM_LICENSE = MIT
PICOM_LICENSE_FILES = COPYING
PICOM_DEPENDENCIES = libev libconfig uthash dbus

ifeq ($(BR2_PACKAGE_HAS_LIBGL),y) 
PICOM_CONF_OPTS += -Dopengl=true
else
PICOM_CONF_OPTS += -Dopengl=false
endif

$(eval $(meson-package))
