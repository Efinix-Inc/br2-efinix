#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "i2c.h"

#define IMX708_I2C_ADDRESS				0x1A
#define DW9807_I2C_ADDRESS				0x0C
#define IMX708_REG_EXPOSURE				0x0202
#define IMX708_REG_DIGITAL_GAIN			0x020e

#define DW9807_CTL_ADDR		0x02
#define DW9807_MSB_ADDR		0x03
#define DW9807_LSB_ADDR		0x04

int set_camera_setting(uint16_t addr, uint16_t regs, uint8_t data);
int camera_set_digital_gain(uint8_t data);
int camera_set_exposure(uint8_t data);
int set_actuator(uint8_t data);
int read_camera_data();
int set_camera_focus(uint16_t data);