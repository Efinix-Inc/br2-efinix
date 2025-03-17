################################################################################
#
# viewnior
#
################################################################################

VIEWNIOR_VERSION = 1.8
VIEWNIOR_DESCRIPTION = Elegant Image Viewer
VIEWNIOR_SITE = $(call github,hellosiyan,Viewnior,viewnior-$(VIEWNIOR_VERSION))
VIEWNIOR_LICENSE = GPL-3.0
VIEWNIOR_LICENSE_FILES = COPYING
VIEWNIOR_DEPENDENCIES += exiv2 shared-mime-info

$(eval $(meson-package))
