##############################################
#
# Hello world
#
##############################################

HELLOWORLD_VERSION = 1.0
HELLOWORLD_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/kernel_modules/helloworld/src
HELLOWORLD_SITE_METHOD = local

define HELLOWORLD_BUILD_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define HELLOWORLD_INSTALL_TARGET_CMDS
	$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules_install
endef

$(eval $(kernel-module))
$(eval $(generic-package))
