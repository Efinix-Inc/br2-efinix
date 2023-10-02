##############################################
#
# socket_hello_world
#
##############################################

SOCKET_HELLO_WORLD_VERSION = 1.0
SOCKET_HELLO_WORLD_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/socket_hello_world/src
SOCKET_HELLO_WORLD_SITE_METHOD = local

define SOCKET_HELLO_WORLD_BUILD_CMDS
        $(MAKE) $(LINUX_MAKE_FLAGS) -C $(@D)
endef

define SOCKET_HELLO_WORLD_INSTALL_TARGET_CMDS
        $(INSTALL) -m 0755 -D $(@D)/socket_hello_world $(TARGET_DIR)/usr/bin/socket_hello_world
endef

$(eval $(generic-package))
