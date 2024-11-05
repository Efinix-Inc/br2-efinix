################################################################################
#
# flops
#
################################################################################

FLOPS_VERSION = 2.2
FLOPS_DESCRIPTION = benchmark floating-point 'MFLOPS' rating for the FADD, FSUB, FMUL, and FDIV operations based on specific 'instruction mixes'
FLOPS_SITE = $(call github,AMDmi3,flops,$(FLOPS_VERSION))
FLOPS_LICENSE = MIT

define FLOPS_BUILD_CMDS
	$(MAKE) CC=$(TARGET_CC) -C $(@D)
endef

define FLOPS_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/flops $(TARGET_DIR)/usr/bin/flops
endef

$(eval $(generic-package))
