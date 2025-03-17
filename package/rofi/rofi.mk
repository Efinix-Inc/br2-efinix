################################################################################
#
# rofi
#
################################################################################

ROFI_VERSION = 1.7.3
ROFI_DESCRIPTION = A window switcher, application launcher and dmenu replacement
ROFI_SITE = $(call github,davatorium,rofi,$(ROFI_VERSION))
ROFI_LICENSE = MIT
ROFI_LICENSE_FILES = COPYING
ROFI_AUTORECONF = YES
ROFI_DEPENDENCIES += startup-notification libxkbcommon

LIBGWATER_VERSION = 555fa6df92434c1c3c7548b5a583b1d8ec3fabb3
LIBGWATER_SITE = https://github.com/sardemff7/libgwater/archive
LIBGWATER_FILE = $(LIBGWATER_VERSION).zip

LIBNKUTILS_VERSION = d08fa898d71da4c11653284968ec14384dd70b6a
LIBNKUTILS_SITE = https://github.com/sardemff7/libnkutils/archive
LIBNKUTILS_FILE = $(LIBNKUTILS_VERSION).zip

ROFI_EXTRA_DOWNLOADS = \
	$(LIBGWATER_SITE)/$(LIBGWATER_FILE) \
	$(LIBNKUTILS_SITE)/$(LIBNKUTILS_FILE)

ROFI_SUBPROJECTS_DIR = $(@D)/subprojects

define ROFI_EXTRACT_SUBMODULE_LIBGWATER
	rm -rf $(ROFI_SUBPROJECTS_DIR)/libgwater && \
	$(UNZIP) -q $(ROFI_DL_DIR)/$(LIBGWATER_FILE) -d $(ROFI_SUBPROJECTS_DIR) && \
	mv $(ROFI_SUBPROJECTS_DIR)/libgwater-$(LIBGWATER_VERSION) $(ROFI_SUBPROJECTS_DIR)/libgwater
endef

define ROFI_EXTRACT_SUBMODULE_LIBNKUTILS
	rm -rf $(ROFI_SUBPROJECTS_DIR)/libnkutils && \
	$(UNZIP) -q $(ROFI_DL_DIR)/$(LIBNKUTILS_FILE) -d $(ROFI_SUBPROJECTS_DIR) && \
	mv $(ROFI_SUBPROJECTS_DIR)/libnkutils-$(LIBNKUTILS_VERSION) $(ROFI_SUBPROJECTS_DIR)/libnkutils
endef


ROFI_CONF_OPTS += \
	--disable-check

ROFI_POST_EXTRACT_HOOKS += \
	ROFI_EXTRACT_SUBMODULE_LIBGWATER \
	ROFI_EXTRACT_SUBMODULE_LIBNKUTILS

$(eval $(autotools-package))

