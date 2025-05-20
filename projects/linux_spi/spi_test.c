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

int spi_test()
{
	char *device = "/dev/spidev1.1";

	int fd = spi_open(device, 0, 8, 5000000);
	if (fd < 0)
	{
		printf("Invalid SPI device file:%s.\n", device);
		return -1;
	}
	else
		printf("SPI: spi device is opened successfully.\n");

	char buff_tx[10];
	char buff_rx[10];

	memset(buff_tx, 0xa6, 10);

	if(spi_transfer(fd, buff_tx, buff_rx, 4)<0)
	{
		printf("SPI: failed to transfer\n");
		return -1;
	}
	else
		printf("SPI: spi transfer is done successfully.\n");

	spi_close(fd);

    return 0;
}

