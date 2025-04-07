/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 *   Nick Kossifidis <mick@ics.forth.gr>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_platform.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/uart8250.h>
#include <sbi_utils/sys/clint.h>
#include "soc.h"

/* clang-format off */
#ifndef SYSTEM_CORES_COUNT
#define VEX_HART_COUNT  1
#else
#define VEX_HART_COUNT  SYSTEM_CORES_COUNT
#endif

#define VEX_PLATFORM_FEATURES  \
	(SBI_PLATFORM_HAS_TIMER_VALUE | SBI_PLATFORM_HAS_MFAULTS_DELEGATION)

#define VEX_HART_STACK_SIZE		4096


#define UART_DATA 0x00
#define UART_STATUS 0x04
#define VEX_CLINT_ADDR SYSTEM_CLINT_CTRL
#define VEX_UART_ADDR ((void*)SYSTEM_UART_0_IO_CTRL)


/* clang-format on */

static struct clint_data clint = {VEX_CLINT_ADDR, 0, VEX_HART_COUNT, true};

static int vex_final_init(bool cold_boot)
{
	return 0;
}

void vex_putc(char ch){
	while(((readl(VEX_UART_ADDR + UART_STATUS) >> 16) & 0xFF) == 0);
	writel(ch, VEX_UART_ADDR);
}

int vex_getc(void){
    return (readl(VEX_UART_ADDR + UART_STATUS) >> 24) == 0 ? -1 : readl(VEX_UART_ADDR + UART_DATA);
}

static int vex_console_init(void)
{
	return 0;
}

static int vex_irqchip_init(bool cold_boot)
{
	return 0;
}

static int vex_ipi_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = clint_cold_ipi_init(&clint);
		if (rc)
			return rc;
	}

	return clint_warm_ipi_init();
}

static int vex_timer_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = clint_cold_timer_init(&clint, NULL);
		if (rc)
			return rc;
	}

	return clint_warm_timer_init();
}

static int vex_system_reset_check(u32 type, u32 reason)
{
	return 0;
}

static void vex_system_reset(u32 type, u32 reason)
{
	/* Tell the "finisher" that the simulation
	 * was successful so that QEMU exits
	 */
}

const struct sbi_platform_operations platform_ops = {
	.final_init		    = vex_final_init,
	.console_putc		= vex_putc,
	.console_getc		= vex_getc,
	.console_init		= vex_console_init,
	.irqchip_init		= vex_irqchip_init,
	.ipi_send		    = clint_ipi_send,
	.ipi_clear		    = clint_ipi_clear,
	.ipi_init		    = vex_ipi_init,
	.timer_value		= clint_timer_value,
	.timer_event_stop	= clint_timer_event_stop,
	.timer_event_start	= clint_timer_event_start,
	.timer_init		= vex_timer_init,
	.system_reset_check	= vex_system_reset_check,
	.system_reset 		= vex_system_reset
};

const struct sbi_platform platform = {
	.opensbi_version	= OPENSBI_VERSION,
	.platform_version	= SBI_PLATFORM_VERSION(0x0, 0x01),
	.name			    = "Efinix Titanium Ti375N1156",
	.features		    = VEX_PLATFORM_FEATURES,
	.hart_count		    = VEX_HART_COUNT,
	.hart_stack_size	= VEX_HART_STACK_SIZE,
	.platform_ops_addr	= (unsigned long)&platform_ops
};


