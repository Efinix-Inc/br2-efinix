#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <unistd.h>

#define MAX_GPIO	3
unsigned int period_us = 200000;  // 200 ms
unsigned int pulse = 20;

int configure_gpio_lines(struct gpiod_chip *chip, struct gpiod_line **lines, unsigned int line_offset)
{
	int i;

	for (i = 0; i < MAX_GPIO; i++) {
		lines[i] = gpiod_chip_get_line(chip, line_offset);
		if (!lines[i]) {
			printf("Failed to get line offset %u\n", line_offset);
			return 1;
		}

		if (gpiod_line_request_output(lines[i], "test", 1) < 0) {
			printf("GPIO line %d is busy\n", i);
			return 1;
		}
		line_offset++;
	}

	return 0;
}

void blinky(struct gpiod_line **lines)
{
	int i, j;

	printf("blinky...\n");
	for (j = 0; j < pulse; j++) {
		for (i = 0; i < MAX_GPIO; i++) {
			gpiod_line_set_value(lines[i], 0);
		}
		usleep(period_us / 2);
		for (i = 0; i < MAX_GPIO; i++) {
			gpiod_line_set_value(lines[i], 1);
		}
		usleep(period_us / 2);
	}
}

void running_light(struct gpiod_line **lines)
{
	int i, j;

	printf("running light...\n");
	for (j = 0; j < pulse; j++) {
		for (i = 0; i < MAX_GPIO; i++) {
			gpiod_line_set_value(lines[i], 0);
			usleep(period_us / 2);
		}

		for (i = (MAX_GPIO -1); i >= 0; i--) {
			gpiod_line_set_value(lines[i], 1);
			usleep(period_us / 2);
		}
	}
}

int main(void)
{
	const char *chip_path = "/dev/gpiochip0";
	unsigned int line_offset = 1;
	struct gpiod_chip *chip;
	struct gpiod_line *lines[MAX_GPIO];
	int ret, i;

	chip = gpiod_chip_open(chip_path);
	if (!chip) {
		printf("Failed to open %s\n", chip_path);
		return 1;
	}

	ret = configure_gpio_lines(chip, lines, line_offset);
	if (ret) {
		gpiod_chip_close(chip);
		return ret;
	}

	blinky(lines);
	running_light(lines);

	gpiod_chip_close(chip);

	return 0;
}
