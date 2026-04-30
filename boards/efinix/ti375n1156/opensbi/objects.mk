# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =
platform-objs-y += platform.o
# Command for platform specific "make run"
platform-runcmd = echo Miaouuuuu


ARCH_BITS := $(shell echo | $(CROSS_COMPILE)gcc -dM -E - < /dev/null | grep __riscv_xlen | awk '{print $$3}')

ifeq ($(ARCH_BITS), 64)
    PLATFORM_RISCV64 := y
    PLATFORM_RISCV_XLEN = 64
    PLATFORM_RISCV_ABI = lp64
    PLATFORM_RISCV_ISA = rv64ima_zicsr_zifencei

    FW_TEXT_START=0x802000000
    FW_JUMP_ADDR=0x802040000
else
    PLATFORM_RISCV32 := y
    PLATFORM_RISCV_XLEN = 32
    PLATFORM_RISCV_ABI = ilp32
    PLATFORM_RISCV_ISA = rv32ima_zicsr_zifencei
    
    AFLAGS += -DPLATFORM_HAVE_MSTATUSH=0
    AFLAGS += -DHAVE_HEXT=0
    CFLAGS += -DPLATFORM_HAVE_MSTATUSH=0 -DHAVE_HEXT=0

    FW_TEXT_START=0x02000000
    FW_JUMP_ADDR=0x02040000
endif

PLATFORM_RISCV_CODE_MODEL = medany

# Blobs to build
FW_DYNAMIC=y
FW_PAYLOAD=n
FW_JUMP=y
