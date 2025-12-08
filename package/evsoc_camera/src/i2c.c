#include "i2c.h"

int i2c_read(struct i2c_device *dev, uint8_t *buf, size_t buf_len)
{
        return read(dev->fd, buf, buf_len);
}

int i2c_write(struct i2c_device *dev, uint8_t *buf, size_t buf_len)
{
        return write(dev->fd, buf, buf_len);
}

int i2c_read_reg8(struct i2c_device *dev, uint8_t *reg, uint8_t *buf, size_t buf_len)
{
        int rc;

        rc = i2c_write(dev, reg, 1);
        if (rc <= 0) {
                 printf("%s: failed to write i2c register address (2 bytes)\n", __func__);
                 return rc;
        }

        rc = i2c_read(dev, buf, buf_len);
        if (rc <= 0) {
                printf("%s: failed to read i2c register data\n", __func__);
                return rc;
        }

        return rc;
}

int i2c_readn(struct i2c_device *dev, uint8_t *reg, uint8_t *buf, size_t buf_len)
{
        int rc;

        rc = i2c_writen(dev, reg, buf, buf_len);
        if (rc) {
            return rc;
        }

        rc = i2c_read(dev, buf, buf_len);
        if (rc <= 0) {
                printf("%s: failed to read i2c register data\n", __func__);
                return rc;
        }

        return rc;
}

int i2c_writen_reg(struct i2c_device *dev, uint8_t reg, uint8_t *buf, size_t buf_len)
{
        uint8_t *full_buf;
        int full_buf_len;
        int rc;
        unsigned int i;

        full_buf_len = buf_len + 1;
        full_buf = (uint8_t *)malloc(sizeof(*buf) * full_buf_len);

        full_buf[0] = reg;
        for (i = 0; i < buf_len; i++) {
                full_buf[i+1] = buf[i];
        }

        rc = i2c_write(dev, full_buf, full_buf_len);
        if (rc <= 0)
                printf("%s: failed to write to i2c register address and data\n", __func__);

        free(full_buf);

        return rc;
}

int i2c_writen(struct i2c_device *dev, uint8_t *reg, uint8_t *buf, int buf_len)
{
        uint8_t *full_buf;
        int full_buf_len;
        int rc = 0;
        int i;

        full_buf_len = buf_len + 2;
        full_buf = (uint8_t *)malloc(sizeof(uint8_t) * full_buf_len);

        // use 2 bytes command
        full_buf[0] = reg[0];
        full_buf[1] = reg[1];
        for (i = 0; i < buf_len; i++) {
                full_buf[i+2] = buf[i];
        }

        rc = i2c_write(dev, full_buf, full_buf_len);
        if (rc <= 0)
                printf("%s: failed to write to i2c register address and data\n", __func__);

        free(full_buf);

        return rc;
}

int i2c_start(struct i2c_device *dev)
{
        int fd;
        int rc;
	char buf;

        fd = open(dev->filename, O_RDWR);
        if (fd < 0) {
                rc = fd;
                goto fail_open;
        }

        /* Set the given I2C slave address */
        rc = ioctl(fd, I2C_SLAVE, dev->addr);
        if (rc < 0) {
                goto fail_set_i2c_slave;
	} else {
		/* Try a quick read 1 byte */
		if (read(fd, &buf, 1) != 1) {
			rc = 1;
			goto fail_set_i2c_slave;
		}
	}

        dev->fd = fd;

        return 0;

fail_set_i2c_slave:
        close(fd);
fail_open:
        return rc;
}
