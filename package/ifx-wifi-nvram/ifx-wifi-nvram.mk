################################################################################
#
# ifx-wifi-nvram
#
################################################################################

IFX_WIFI_NVRAM_VERSION = 411c87d4cf924a1a5415273265fd54d7d7d4044f
IFX_WIFI_NVRAM_SITE = https://github.com/murata-wireless/cyw-fmac-nvram.git
IFX_WIFI_NVRAM_SITE_METHOD = git
IFX_WIFI_NVRAM_LICENSE = PROPRIETARY
IFX_WIFI_NVRAM_LICENSE_FILES = LICENSE

# Required for no-build package
#define IFX_WIFI_NVRAM_BUILD_CMDS
#	@true
#endef

define IFX_WIFI_NVRAM_INSTALL_TARGET_CMDS
	$(INSTALL) -d $(TARGET_DIR)/lib/firmware/cypress
	$(INSTALL) -m 0644 \
		$(@D)/cyfmac5557x-pcie_sdio.sant.2EA_2EC.txt \
		$(TARGET_DIR)/lib/firmware/cypress/cyfmac55572-sdio.txt
endef

$(eval $(generic-package))

