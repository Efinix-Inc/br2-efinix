##############################################
#
# emmc_programmer
#
##############################################

EMMC_PROGRAMMER_VERSION = 1.0
EMMC_PROGRAMMER_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/emmc_programmer/src
EMMC_PROGRAMMER_SITE_METHOD = local
EMMC_PROGRAMMER_DEPENDENCIES = python3

define EMMC_PROGRAMMER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/emmc_programmer.py $(TARGET_DIR)/usr/bin/emmc_programmer
endef

$(eval $(generic-package))