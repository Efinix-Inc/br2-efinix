# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Command for platform specific "make run"
platform-runcmd = echo Miaouuuuu

PLATFORM_RISCV_XLEN = 32
PLATFORM_RISCV_ABI = ilp32
PLATFORM_RISCV_ISA = rv32ima
PLATFORM_RISCV_CODE_MODEL = medany

# Blobs to build
FW_TEXT_START=0x01000000
FW_DYNAMIC=y
FW_PAYLOAD=n
FW_JUMP=y
#FW_JUMP_ADDR=0x00F80000
FW_JUMP_ADDR=0x01040000
FW_JUMP_FDT_ADDR=0x01F00000
