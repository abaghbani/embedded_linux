#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>

#include "spi.h"

int spi_open(struct SPIDevice* dev)
{
	dev->fd = open(dev->filename, O_RDWR | O_NONBLOCK);
	if (dev->fd < 0)
	{
		printf("failed to open spi device: %s\n", dev->filename);
		return 0;
	}
	
	if(ioctl(dev->fd, SPI_IOC_WR_MODE32, &(dev->mode)) < 0)
		return 0;
	if(ioctl(dev->fd, SPI_IOC_RD_MODE32, &(dev->mode)) < 0)
		return 0;
	if(ioctl(dev->fd, SPI_IOC_WR_BITS_PER_WORD, &(dev->bits)) < 0)
		return 0;
	if(ioctl(dev->fd, SPI_IOC_RD_BITS_PER_WORD, &(dev->bits)) < 0)
		return 0;
	if(ioctl(dev->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(dev->speed)) < 0)
		return 0;
	if(ioctl(dev->fd, SPI_IOC_RD_MAX_SPEED_HZ, &(dev->speed)) < 0)
		return 0;

	return 1;
}

ssize_t spi_transfer(struct SPIDevice* dev, void *out, void *in, size_t len)
{
	struct spi_ioc_transfer buff;

	memset(&buff, 0, sizeof(buff));
	buff.tx_buf=(__u64)out;
	buff.rx_buf=(__u64)in;
	buff.len=len;
	buff.delay_usecs=0;
	if(ioctl(dev->fd, SPI_IOC_MESSAGE(1), &buff) < 0)
		return 0;
	return len;
}

int spi_close(struct SPIDevice* dev)
{
	return close(dev->fd);
}
