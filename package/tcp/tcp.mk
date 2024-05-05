##############################################
#
# tcp
#
##############################################

TCP_VERSION = 1.0
TCP_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/tcp/src
TCP_SITE_METHOD = local

define TCP_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define TCP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/tcp_client $(TARGET_DIR)/usr/bin/tcp_client
	$(INSTALL) -m 0755 -D $(@D)/tcp_server $(TARGET_DIR)/usr/bin/tcp_server
endef

$(eval $(generic-package))
