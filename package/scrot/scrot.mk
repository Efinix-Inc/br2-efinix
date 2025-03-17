################################################################################
#
# scrot
#
################################################################################

SCROT_VERSION = v0.10.0
SCROT_DESCRIPTION = scrot (SCReen shOT) is a simple commandline screen capture utility
SCROT_SITE = $(call github,dreamer,scrot,$(SCROT_VERSION))
SCROT_LICENSE = MIT
SCROT_LICENSE_FILES = COPYING
SCROT_DEPENDENCIES = imlib2 giblib

$(eval $(meson-package))
