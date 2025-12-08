#include "cam_ops.h"

int check_camera_i2c_bus(struct i2c_device *i2c_dev)
{
	char filename[32];
	int ret, i2c_bus;

	/* Scan first 3 i2c buses */
	for (i2c_bus = 0; i2c_bus < 3; i2c_bus++) {
		sprintf(filename, "/dev/i2c-%d", i2c_bus);
		i2c_dev->filename = filename;
		ret = i2c_start(i2c_dev);
		if (ret == 0) {
			printf("Camera is using i2c bus %d, %s\n", i2c_bus, i2c_dev->filename);
			i2c_dev->i2c_bus = i2c_bus;
			return 0;
		}
	}

	return 1;
}

int set_camera_setting(uint16_t addr, uint16_t regs, uint8_t data)
{
	struct i2c_device i2c_dev;
	int ret;
	uint8_t reg[2];

	i2c_dev.addr = addr;
	ret = check_camera_i2c_bus(&i2c_dev);
	if (ret) {
		printf("Failed to initialize i2c device on bus %d on %s\n", i2c_dev.i2c_bus, i2c_dev.filename);
		return ret;
	}

	reg[0] = (regs >> 8) & 0xFF;
	reg[1] = regs & 0xFF;

	ret = i2c_writen(&i2c_dev, reg, &data, 1);
	if (ret <= 0) {
		printf("i2c write failed\n");
		return ret;
	}

	return 0;
}

int camera_set_digital_gain(uint8_t data)
{
	int ret;
	uint16_t addr = IMX708_I2C_ADDRESS;
	uint16_t regs = IMX708_REG_DIGITAL_GAIN;

	ret = set_camera_setting(addr, regs, data);
	if (ret)
		return ret;

	return 0;
}

int camera_set_exposure(uint8_t data)
{
	int ret;
	uint16_t addr = IMX708_I2C_ADDRESS;
	uint16_t regs = IMX708_REG_EXPOSURE;

	ret = set_camera_setting(addr, regs, data);
	if (ret)
		return ret;

	return 0;
}

int set_actuator(uint8_t data)
{
        int ret;
        struct i2c_device dev;
        char filename[30] = "/dev/i2c-0";
        uint8_t reg = 0x02;

        dev.addr = 0x0c;
        dev.filename = filename;

        ret = i2c_start(&dev);
        if (ret) {
                printf("failed to start i2c on 0x%x\n", dev.addr);
        }

        ret = i2c_writen_reg(&dev, reg, &data, 1);
        if (ret)
                return ret;

        return 0;
}

int set_camera_focus(uint16_t data)
{
        int ret;
        struct i2c_device dev;
        char filename[30] = "/dev/i2c-0";
        uint8_t reg[2];
        uint8_t temp[2];

        dev.addr = 0x0c;
        dev.filename = filename;

        ret = i2c_start(&dev);
        if (ret <= 0) {
                printf("failed to start i2c on 0x%x\n", dev.addr);
        }

        reg[0] = 0x03;
        reg[1] = 0x04;

        temp[0] = (data >> 8) & 0x03;
        temp[1] = data & 0xff;

        ret = i2c_writen(&dev, reg, temp, 2);
        if (ret <= 0) {
                printf("i2c write failed\n");
                return ret;
        }

        return 0;
}

int read_camera_data()
{
	struct i2c_device dev;
	int ret;
	uint8_t reg[2];
	uint8_t i2c_node_address = 0;
	uint8_t buf = 0;
	char filename[32];

	reg[0] = IMX708_REG_EXPOSURE;
	reg[1] = IMX708_REG_EXPOSURE >> 8;
	dev.addr = IMX708_I2C_ADDRESS;

	sprintf(filename, "/dev/i2c-%d", i2c_node_address);
	dev.filename = filename;
	ret = i2c_start(&dev);
	if (ret) {
		printf("Failed to initialize /dev/i2c-%d\n", i2c_node_address);
		return ret;
	}

	ret = i2c_readn(&dev, reg, &buf, 1);
	if (ret)
		return ret;

	printf("exposure = 0x%x\n", buf);

	return 0;
}
