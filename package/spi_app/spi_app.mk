################################################################################
#
# spi_app
#
################################################################################

SPI_APP_VERSION = 1.0
SPI_APP_DESCRIPTION = sample spi application
SPI_APP_SITE_METHOD = local
SPI_APP_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/spi_app/src
SPI_APP_LICENSE = MIT

define SPI_APP_BUILD_CMDS
	$(MAKE) CC=$(TARGET_CC) -C $(@D)
endef

define SPI_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/spi_app $(TARGET_DIR)/usr/bin/spi_app
endef

$(eval $(generic-package))
