#ifndef GPIO_H_
#define GPIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define SYS_GPIO_PATH "/sys/class/efx_gpios"
#define SOME_BYTES 100

#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1

#define GPIO_LED1 "LED1"
#define GPIO_LED2 "LED2"
#define GPIO_LED3 "LED3"

int gpio_configure_dir(char *gpio_label, uint8_t dir_value);
int gpio_set_value(char *gpio_label, uint8_t value);

#endif /* end GPIO_H_ */
