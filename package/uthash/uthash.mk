################################################################################
#
# uthash
#
################################################################################

UTHASH_VERSION = f69112c04f1b6e059b8071cb391a1fcc83791a00
UTHASH_DESCRIPTION = C macros for hash tables and more
UTHASH_SITE = $(call github,troydhanson,uthash,$(UTHASH_VERSION))
UTHASH_INSTALL_TARGET = YES
UTHASH_LICENSE = MIT
UTHASH_LICENSE_FILES = LICENSE

define UTHASH_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 644 $(@D)/src/utarray.h $(STAGING_DIR)/usr/include/utarray.h
	$(INSTALL) -D -m 644 $(@D)/src/uthash.h $(STAGING_DIR)/usr/include/uthash.h
	$(INSTALL) -D -m 644 $(@D)/src/utlist.h $(STAGING_DIR)/usr/include/utlist.h
	$(INSTALL) -D -m 644 $(@D)/src/utringbuffer.h $(STAGING_DIR)/usr/include/utringbuffer.h
	$(INSTALL) -D -m 644 $(@D)/src/utstack.h $(STAGING_DIR)/usr/include/utstack.h
	$(INSTALL) -D -m 644 $(@D)/src/utstring.h $(STAGING_DIR)/usr/include/utstring.h
	
endef

$(eval $(generic-package))
