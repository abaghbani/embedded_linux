/*
 * i2c_test.c
 *
 *  Created on: Sep 22, 2021
 *      Author: Akbar
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "i2c.h"

//i2c test project
int main()
{
    struct I2cDevice dev;

	dev.filename = "/dev/i2c-0";
	dev.addr = 0x44;

	if(i2c_start(&dev) < 0)
	{
		printf("failed to start i2c device\r\n");
		return -1;
	}

	uint8_t tx_buf[10];
	tx_buf[0] = 0x01;
	tx_buf[1] = 0x02;
	tx_buf[2] = 0x03;
	if(i2c_write(&dev, tx_buf, 3) < 0)
	{
		printf("failed to write into i2c device: %x, there is a NAK, (this address is not valid)\r\n", dev.addr);
		//return -1;
	}

	i2c_stop(&dev);

    return 0;
}

