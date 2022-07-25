#pragma once

#include "bsp.h"
#include "io.h"
#include "spiFlash.h"
#include "riscv.h"

#define SPI SYSTEM_SPI_0_IO_CTRL
#define SPI_CS 0

#define OPENSBI_MEMORY    0x01000000
#define OPENSBI_FLASH     0x00400000
#define OPENSBI_SIZE      0x040000

#define UBOOT_MEMORY      0x01040000
#define UBOOT_SBI_FLASH   0x00480000
#define UBOOT_SIZE        0x0C0000

void bspMain()
{
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
