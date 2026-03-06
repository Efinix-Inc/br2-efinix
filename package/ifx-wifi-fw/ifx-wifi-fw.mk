################################################################################
#
# ifx-wifi-fw
#
################################################################################

IFX_WIFI_FW_VERSION = c6c73c37e89b088624f7970fb0a7ef535c5dee0b
IFX_WIFI_FW_SITE = https://github.com/Infineon/ifx-linux-firmware.git
IFX_WIFI_FW_SITE_METHOD = git
IFX_WIFI_FW_LICENSE = PROPRIETARY
IFX_WIFI_FW_LICENSE_FILES = LICENSE

define IFX_WIFI_FW_INSTALL_TARGET_CMDS
	$(INSTALL) -d $(TARGET_DIR)/lib/firmware/cypress
	$(INSTALL) -m 0644 $(@D)/firmware/cyfmac55572-sdio.clm_blob \
		$(TARGET_DIR)/lib/firmware/cypress/
	$(INSTALL) -m 0644 $(@D)/firmware/cyfmac55572-sdio.trxse \
		$(TARGET_DIR)/lib/firmware/cypress/
endef

$(eval $(generic-package))
