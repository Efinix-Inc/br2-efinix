################################################################################
#
# picom
#
################################################################################

PICOM_VERSION = v9
PICOM_DESCRIPTION = A lightweight compositor for X11 with animation support
PICOM_SITE = $(call github,yshui,picom,$(PICOM_VERSION))
PICOM_LICENSE = MIT
PICOM_LICENSE_FILES = COPYING
PICOM_DEPENDENCIES = libev libconfig uthash dbus

ifeq ($(BR2_PACKAGE_HAS_LIBGL),y) 
PICOM_CONF_OPTS += -Dopengl=true
else
PICOM_CONF_OPTS += -Dopengl=false
endif

$(eval $(meson-package))
