//*------------------------------------------------------------------------------------------
//  MIT License
//  
//  Copyright (c) 2021 SaxonSoc contributors
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//*-----------------------------------------------------------------------------------------
#pragma once

#include "bsp.h"
#include "io.h"
#include "spiFlash.h"
#include "riscv.h"

#define SPI SYSTEM_SPI_0_IO_CTRL
#define SPI_CS 0

#define OPENSBI_MEMORY		0x01000000
#define OPENSBI_FLASH		0X400000
#define OPENSBI_SIZE		0X040000

#define UBOOT_MEMORY		0x01040000
#define UBOOT_FLASH		0x480000
#define UBOOT_SIZE		0x0C0000

void bspMain() {
#ifndef SIM
	spiFlash_init(SPI, SPI_CS);
	spiFlash_wake(SPI, SPI_CS);
	bsp_putString("OpenSBI copy\n");
	spiFlash_f2m(SPI, SPI_CS, OPENSBI_FLASH, OPENSBI_MEMORY, OPENSBI_SIZE);
	bsp_putString("U-Boot copy\n");
        spiFlash_f2m(SPI, SPI_CS, UBOOT_FLASH, UBOOT_MEMORY, UBOOT_SIZE);
#endif

	bsp_putString("Payload boot\n");
	void (*userMain)(u32, u32, u32) = (void (*)(u32, u32, u32))OPENSBI_MEMORY;
	userMain(0,0,0);
}
