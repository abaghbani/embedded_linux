/*
 * spi_test.c
 *
 *  Created on: Sep 22, 2021
 *      Author: Akbar
 */



#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "spi.h"

int main()
{
	struct SPIDevice device = {
		.filename = "/dev/spidev1.1",
		.mode = 0,
		.bits = 8,
		.speed = (uint32_t)(5000000)
	};
	
	if(spi_open(&device)==0)
	{
		printf("Invalid SPI device file:%s.\n", device.filename);
		return -1;
	}
	else
		printf("SPI: spi device is opened successfully.\n");

	char buff_tx[10];
	char buff_rx[10];

	memset(buff_tx, 0xa6, 10);

	if(spi_transfer(&device, buff_tx, buff_rx, 4)==0)
	{
		printf("SPI: failed to transfer\n");
		return -1;
	}
	else
		printf("SPI: spi transfer is done successfully.\n");

	spi_close(&device);

    return 0;
}

