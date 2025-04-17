################################################################################
#
# linpack
#
################################################################################

LINPACK_VERSION = 1.0
LINPACK_DESCRIPTION = LINPACK Benchmarks are a measure of a system's floating point computing power
LINPACK_SITE = https://www.netlib.org/benchmark
LINPACK_SOURCE = linpackc.new
LINPACK_TARGET = linpacknew.c
LINPACK_LICENSE = PD

define LINPACK_DOWNLOAD
	$(call DOWNLOAD,$(LINPACK_SITE)/$(LINPACK_SOURCE))
endef

define LINPACK_EXTRACT_CMDS
	cp $(DL_DIR)/linpack/$(LINPACK_SOURCE) $(@D)/$(LINPACK_TARGET)
endef

define LINPACK_BUILD_CMDS
	cd $(@D) && \
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -DDP -o linpack_dp linpacknew.c -lm && \
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -DSP -o linpack_sp linpacknew.c -lm
endef

define LINPACK_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/linpack_dp $(TARGET_DIR)/usr/bin/linpack_dp
	$(INSTALL) -m 0755 -D $(@D)/linpack_sp $(TARGET_DIR)/usr/bin/linpack_sp
endef

$(eval $(generic-package))
