#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "i2c.h"

int main()
{
    struct I2cDevice dev;

	dev.filename = "/dev/i2c-1";
	dev.addr = 0x30;

	if(i2c_start(&dev) < 0)
	{
		printf("failed to start i2c device: %x\r\n", dev.addr);
		return -1;
	}

	uint8_t buf[10];
	i2c_write_reg(&dev, 0x01, 0x01);
	buf[0] = i2c_read_reg(&dev, 0x06);
	printf("read from i2c device: %x, reg-add: %x, value: %x \r\n", dev.addr, 0x06, buf[0]);

	i2c_stop(&dev);

    return 0;
}

