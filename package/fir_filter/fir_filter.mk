##############################################
#
# fir_filter
#
##############################################

FIR_FILTER_VERSION = 1.0
FIR_FILTER_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/fir_filter/src
FIR_FILTER_SITE_METHOD = local

define FIR_FILTER_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define FIR_FILTER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/fir_filter $(TARGET_DIR)/usr/bin/fir_filter

endef

$(eval $(generic-package))
