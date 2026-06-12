#pragma once

#include "bsp.h"
#include "io.h"
#include "start.h"

#if __riscv_xlen == 64 //RV64

#include "spiFlash/spiFlash.h"

#define OPENSBI_MEMORY  0x802000000
#define OPENSBI_FLASH   0x00600000
#define OPENSBI_SIZE    0x040000

#define UBOOT_MEMORY    0x802040000
#define UBOOT_SBI_FLASH 0x00680000
#define UBOOT_SIZE      0x0C0000

#define SPI             (spi_hwreg_t* )SYSTEM_SPI_0_IO_CTRL

spiFlash_instance_t Flash = {
    .cs = 0,
    .inst = &(spi_instance_t){
        .hwreg      = SPI,
        .cpol       = LOW,
        .cpha       = DATA_SAMPLED_RISE_EDGE,
        .mode       = FULL_DUPLEX_SINGLE_LINE
    },
};

#else // RV32

#include "spiFlash.h"

#define SPI SYSTEM_SPI_0_IO_CTRL
#define SPI_CS                  0

#ifdef Ti60F225_DEVKIT
#define OPENSBI_MEMORY          0x01000000
#define UBOOT_MEMORY            0x01040000
#else
#define OPENSBI_MEMORY          0x02000000
#define UBOOT_MEMORY            0x02040000
#endif

#define OPENSBI_FLASH           0x00600000
#define OPENSBI_SIZE            0x040000

#define UBOOT_SBI_FLASH         0x00680000
#define UBOOT_SIZE              0x0C0000

#define UART_0_SAMPLE_PER_BAUD  8
#define UART_0_BAUD_RATE        115200

void configure_uart()
{
    Uart_Config uart0;
    uart0.dataLength = BITS_8;
    uart0.parity = NONE;
    uart0.stop = ONE;
    uart0.clockDivider = BSP_CLINT_HZ/(UART_0_BAUD_RATE * UART_0_SAMPLE_PER_BAUD) - 1;
    uart_applyConfig(BSP_UART_TERMINAL, &uart0);
}

#endif // __riscv_xlen

void bspMain() {
    bsp_printf_s("Built on " __DATE__ " at " __TIME__ "\r\n");

#if __riscv_xlen == 64 // RV64
    bsp_printf_s("RISC-V: 64 bit\r\n");

    // Initialize the flash and read JEDEC ID to verify communication with flash and apply flash specific configuration
    spiFlash_probe(&Flash);
    bsp_printf_s("OpenSBI copy\r\n");
    spiFlash_f2m_single(&Flash, OPENSBI_FLASH, OPENSBI_MEMORY, OPENSBI_SIZE);
    bsp_printf_s("U-Boot copy\r\n");
    spiFlash_f2m_single(&Flash, UBOOT_SBI_FLASH, UBOOT_MEMORY, UBOOT_SIZE);
    bsp_printf_s("Payload boot\r\n");
    void (*userMain)(u64, u64, u64) = (void (*)(u64, u64, u64))OPENSBI_MEMORY;

#else // RV32

    configure_uart();
    bsp_printf_s("RISC-V: 32 bit\r\n");

    spiFlash_init(SPI, SPI_CS);
    spiFlash_wake(SPI, SPI_CS);
    bsp_printf_s("OpenSBI copy\r\n");
    spiFlash_f2m(SPI, SPI_CS, OPENSBI_FLASH, OPENSBI_MEMORY, OPENSBI_SIZE);
    bsp_printf_s("U-Boot copy\r\n");
    spiFlash_f2m(SPI, SPI_CS, UBOOT_SBI_FLASH, UBOOT_MEMORY, UBOOT_SIZE);

    bsp_printf_s("Payload boot\r\n");
    void (*userMain)(u32, u32, u32) = (void (*)(u32, u32, u32))OPENSBI_MEMORY;

#endif // __riscv_xlen

#ifdef SMP
    smp_unlock(userMain);
#endif
    bsp_printf_s("Starting OpenSBI\r\n");
    userMain(0, 0, 0);
}
