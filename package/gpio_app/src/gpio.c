#include "gpio.h"

int gpio_configure_dir(char *gpio_label, uint8_t dir_value)
{
	int fd;
	char buf[SOME_BYTES];

	snprintf(buf, sizeof(buf), SYS_GPIO_PATH "/%s/direction", gpio_label);
	
	/* open sysfs attribute 'direction' */
	fd = open(buf, O_WRONLY | O_SYNC);
	if (fd < 0) {
		printf("GPIO %s configure as %d\n", gpio_label, dir_value);
		return fd;
	}

	/* write to sysfs attribute 'direction' */
	if (dir_value)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

int gpio_set_value(char *gpio_label, uint8_t value)
{
	int fd;
	char buf[SOME_BYTES];

	snprintf(buf, sizeof(buf), SYS_GPIO_PATH "/%s/value", gpio_label);

	fd = open(buf, O_WRONLY | O_SYNC);
        if (fd < 0) {
                printf("GPIO %s configure as %d\n", gpio_label, value);
                return fd;
        }

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}
