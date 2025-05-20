/*
 * i2c.c
 *
 *  Created on: Sep 22, 2021
 *      Author: Akbar
 */


#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "i2c.h"


int i2c_start(struct I2cDevice* dev)
{
	dev->fd = open(dev->filename, O_RDWR);
	if (dev->fd < 0)
	{
		printf("failed to open i2c device: %s\n", dev->filename);
		return -1;
	}

	if(ioctl(dev->fd, I2C_SLAVE_FORCE, dev->addr) < 0)
	{
		printf("failed to set i2c address: %s\n", dev->filename);
		close(dev->fd);
		return -1;
	}
	if(ioctl(dev->fd, I2C_RETRIES, 5) < 0)
	{
		printf("failed to set i2c retry: %s\n", dev->filename);
		close(dev->fd);
		return -1;
	}
	if(ioctl(dev->fd, I2C_TIMEOUT, 2) < 0)
	{
		printf("failed to set i2c timeout: %s\n", dev->filename);
		close(dev->fd);
		return -1;
	}

	return 0;
}

int i2c_read(struct I2cDevice* dev, uint8_t *buf, size_t buf_len)
{
	return read(dev->fd, buf, buf_len);
}

int i2c_write(struct I2cDevice* dev, uint8_t *buf, size_t buf_len)
{
	return write(dev->fd, buf, buf_len);
}

void i2c_stop(struct I2cDevice* dev)
{
	close(dev->fd);
}

int i2c_readn_reg(struct I2cDevice* dev, uint8_t reg, uint8_t *buf, size_t buf_len)
{
	if(i2c_write(dev, &reg, 1) < 0)
	{
		printf("%s: failed to write i2c register address\r\n", __func__);
		return -1;
	}

	if(i2c_read(dev, buf, buf_len) < 0)
	{
		printf("%s: failed to read i2c register data\r\n", __func__);
		return -1;
	}

	return 0;
}

int i2c_writen_reg(struct I2cDevice* dev, uint8_t reg, uint8_t *buf, size_t buf_len)
{
	uint8_t *full_buf;

	full_buf = malloc(sizeof(uint8_t) * buf_len + 1);
	full_buf[0] = reg;
	for (int i = 0; i < buf_len; i++)
	{
		full_buf[i + 1] = buf[i];
	}

	if(i2c_write(dev, full_buf, buf_len + 1) < 0)
	{
		printf("%s: failed to write i2c register address and data\r\n", __func__);
		free(full_buf);
		return -1;
	}

	free(full_buf);
	return 0;
}

uint8_t i2c_read_reg(struct I2cDevice* dev, uint8_t reg)
{
	uint8_t value = 0;
	i2c_readn_reg(dev, reg, &value, 1);
	return value;
}

int i2c_write_reg(struct I2cDevice* dev, uint8_t reg, uint8_t value)
{
	return i2c_writen_reg(dev, reg, &value, 1);
}

int i2c_mask_reg(struct I2cDevice* dev, uint8_t reg, uint8_t mask)
{
	 uint8_t value = i2c_read_reg(dev, reg);
	value |= mask;

	if(i2c_write_reg(dev, reg, value) < 0)
	{
		return -1;
	}

	return 0;
}
