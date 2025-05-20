#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "peripheral.h"

int set_led_dimming(struct I2cDevice *dev, uint8_t led, uint8_t dimming_value)
{
	uint8_t tx_buf[4];
	tx_buf[0] = led;
	tx_buf[1] = dimming_value;
	tx_buf[2] = dimming_value;
	tx_buf[3] = dimming_value;
	if(i2c_write(dev, tx_buf, 4) < 0)
	{
		printf("I2C error: write to i2c is failed\n");
		return 0;
	}
	
	return 1;
}

int set_led_color(struct I2cDevice *dev, uint8_t led, enum led_color color)
{
	uint8_t tx_buf[2];
	tx_buf[0] = led;
	tx_buf[1] = (uint8_t)color;
	if(i2c_write(dev, tx_buf, 2) < 0)
	{
		printf("I2C error: write to i2c is failed\n");
		return 0;
	}
	
	return 1;
}

int set_att(struct SPIDevice *dev, uint8_t att, enum att_value value)
{
	char buff_tx[3];
	buff_tx[0] = att_write_value_command;
	buff_tx[1] = att;
	buff_tx[2] = (uint8_t)value;
	if(!spi_transfer(dev, buff_tx, NULL, 3))
	{
		printf("SPI: failed to transfer\n");
		return 0;
	}
	
	return 1;
}

int set_sc_led(struct I2cDevice *dev)
{
	uint8_t tx_buf[10];
	// LED overwrite mode (blue)  -> {Start} B0 {Start} 40 04 1E AC 0B CD {Stop}
	tx_buf[0] = 0xB0;
	if(i2c_write(dev, tx_buf, 1) < 0)
	{
		printf("failed to write into i2c device: %x, there is a NAK, (this address is not valid)\r\n", dev->addr);
		return 0;
	}
	tx_buf[0] = 0x40;
	tx_buf[1] = 0x04;
	tx_buf[2] = 0x1E;
	tx_buf[3] = 0xAC;
	tx_buf[4] = 0x0B;
	tx_buf[5] = 0xCD;
	if(i2c_write(dev, tx_buf, 6) < 0)
	{
		printf("failed to write into i2c device: %x, there is a NAK, (this address is not valid)\r\n", dev->addr);
		return 0;
	}
	
	return 1;

}

int peripheral_stop(struct peripheral_info * info)
{
	i2c_stop(&(info->i2c_device_p));
	spi_close(&(info->spi_device_p));
	
	return 1;
}

int peripheral_init(struct peripheral_info * info)
{
	// i2c (led interface) initialization
	info->i2c_device_p.filename = linux_i2c_device;
	info->i2c_device_p.addr = i2c_address_front_panel;
	if(i2c_start(&(info->i2c_device_p)) < 0)
	{
		printf("failed to start i2c device: %s\r\n", info->i2c_device_p.filename);
		i2c_stop(&(info->i2c_device_p));
		return 0;
	}
	set_led_dimming(&(info->i2c_device_p), led_operating_dimming_address, 0xf0);
	set_led_dimming(&(info->i2c_device_p), led_activity_dimming_address, 0xf0);
	
	// spi (att interface) initialization
	info->spi_device_p.filename = linux_spi_device;
	info->spi_device_p.mode = 0;
	info->spi_device_p.bits = 8;
	info->spi_device_p.speed = 5000000;
	if(!spi_open(&(info->spi_device_p)))
	{
		printf("failed to start SPI device: %s.\n", info->spi_device_p.filename);
		spi_close(&(info->spi_device_p));
		return 0;
	}
	
	return 1;
}

int peripheral(struct peripheral_info * info)
{
	// set led color
	if(info->led_update)
	{
		set_led_color(&(info->i2c_device_p), led_operating_address, info->led_operating);
		set_led_color(&(info->i2c_device_p), led_activity_address, info->led_activity);
		info->led_update = false;
	}
	
	// set attenuator value:
	if(info->attenuator_update)
	{
		set_att(&(info->spi_device_p), att_address_tx0, info->attenuator);
		set_att(&(info->spi_device_p), att_address_rx0, info->attenuator);
		set_att(&(info->spi_device_p), att_address_rx1, info->attenuator);
		info->attenuator_update = false;
	}
	
	return 1;
}
