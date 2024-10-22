#pragma once

#include "bsp.h"
#include "io.h"
#include "spiFlash.h"
#include "riscv.h"
#include "start.h"

#define SPI SYSTEM_SPI_0_IO_CTRL
#define SPI_CS 0

#define OPENSBI_MEMORY    0x01000000
#define OPENSBI_FLASH     0x00600000
#define OPENSBI_SIZE      0x040000

#define UBOOT_MEMORY      0x01040000
#define UBOOT_SBI_FLASH   0x00680000
#define UBOOT_SIZE        0x0C0000

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


void bspMain()
{
    configure_uart();

#ifndef SPINAL_SIM
    spiFlash_init(SPI, SPI_CS);
    spiFlash_wake(SPI, SPI_CS);
    bsp_putString("OpenSBI copy\r\n");
    spiFlash_f2m(SPI, SPI_CS, OPENSBI_FLASH, OPENSBI_MEMORY, OPENSBI_SIZE);
    bsp_putString("U-Boot copy\r\n");
    spiFlash_f2m(SPI, SPI_CS, UBOOT_SBI_FLASH, UBOOT_MEMORY, UBOOT_SIZE);
#endif

    bsp_putString("Payload boot\r\n");
    void (*userMain)(u32, u32, u32) = (void (*)(u32, u32, u32))OPENSBI_MEMORY;
    #ifdef SMP
    smp_unlock(userMain);
    #endif
    userMain(0, 0, 0);
}
