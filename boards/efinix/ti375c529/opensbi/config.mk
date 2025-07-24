# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Command for platform specific "make run"
platform-runcmd = echo Miaouuuuu


ARCH_BITS := $(shell echo | $(CROSS_COMPILE)gcc -dM -E - < /dev/null | grep __riscv_xlen | awk '{print $$3}')

ifeq ($(ARCH_BITS), 64)
    PLATFORM_RISCV64 := y
    PLATFORM_RISCV_XLEN = 64
    PLATFORM_RISCV_ABI = lp64
    PLATFORM_RISCV_ISA = rv64ima
else
    PLATFORM_RISCV32 := y
    PLATFORM_RISCV_XLEN = 32
    PLATFORM_RISCV_ABI = ilp32
    PLATFORM_RISCV_ISA = rv32ima
endif

PLATFORM_RISCV_CODE_MODEL = medany

# Blobs to build
FW_TEXT_START=0x01000000
FW_DYNAMIC=y
FW_PAYLOAD=n
FW_JUMP=y
#FW_JUMP_ADDR=0x00F80000
FW_JUMP_ADDR=0x01040000
FW_JUMP_FDT_ADDR=0x00CF0000
