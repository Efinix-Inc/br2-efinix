/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <time.h>
#include "gpio.h"



void led_init()
{
	/* set direction to out */
	/* set value to 0 to turn off the LED */
	printf("Initialize LED\n");
	gpio_configure_dir(GPIO_LED1, GPIO_DIR_OUT);
	gpio_configure_dir(GPIO_LED2, GPIO_DIR_OUT);
	gpio_configure_dir(GPIO_LED3, GPIO_DIR_OUT);
}

void gpio_led_off()
{
        gpio_set_value(GPIO_LED1, 0);
        gpio_set_value(GPIO_LED2, 0);
        gpio_set_value(GPIO_LED3, 0);
}

void gpio_led_circle()
{
	int i;
	gpio_led_off();

	printf("GPIO LED circle\n");
	for (i=0; i<10; i++) {
		gpio_set_value(GPIO_LED1, 1);
		sleep(1);
		gpio_set_value(GPIO_LED2, 1);
		sleep(1);
		gpio_set_value(GPIO_LED3, 1);
		sleep(1);
		gpio_set_value(GPIO_LED1, 0);
		sleep(1);
		gpio_set_value(GPIO_LED2, 0);
		sleep(1);
		gpio_set_value(GPIO_LED3, 0);
		sleep(1);
	}
	gpio_led_off();
}

void gpio_led_on()
{
        gpio_set_value(GPIO_LED1, 1);
        gpio_set_value(GPIO_LED2, 1);
        gpio_set_value(GPIO_LED3, 1);
}

void gpio_led_emergency()
{
	printf("gpio LED emergency\n");
	gpio_led_off();
	sleep(1);
	int i;

	for (i=0; i<10; i++)
	{
		gpio_set_value(GPIO_LED1, 1);
		sleep(1);
		gpio_set_value(GPIO_LED3, 1);
		gpio_set_value(GPIO_LED1, 0);
		sleep(1);
		gpio_set_value(GPIO_LED3, 0);
	}
	gpio_led_off();
}

int main()
{
	printf("Toggling on board LED\n");
	led_init();

	while (1)
	{
		gpio_led_circle();
		sleep(1);
		gpio_led_emergency();
		sleep(1);
		gpio_led_on();
		sleep(2);
		gpio_led_off();
		sleep(1);

	}
	return 0;
}
