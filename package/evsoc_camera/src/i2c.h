#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

struct i2c_device {
        char *filename; /* Path of I2C bus. eg. /dev/i2c-0 */
        uint16_t addr; /* Address of I2C slave */
        int fd; /* file descriptor for the I2C bus */
	int i2c_bus;
};

int i2c_read(struct i2c_device *dev, uint8_t *buf, size_t buf_len);
int i2c_write(struct i2c_device *dev, uint8_t *buf, size_t buf_len);
int i2c_read_reg8(struct i2c_device *dev, uint8_t *reg, uint8_t *buf, size_t buf_len);
int i2c_readn(struct i2c_device *dev, uint8_t *reg, uint8_t *buf, size_t buf_len);
int i2c_writen(struct i2c_device *dev, uint8_t *reg, uint8_t *buf, int buf_len);
int i2c_start(struct i2c_device *dev);
int i2c_writen_reg(struct i2c_device *dev, uint8_t reg, uint8_t *buf, size_t buf_len);
