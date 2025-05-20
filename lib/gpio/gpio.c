/*
 * gpio.c
 *
 *  Created on: Sep 22, 2021
 *      Author: Akbar
 */


#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "gpio.h"


uint32_t gpio_open_channel(GPIO_PIN_DAT *p_dat)
{
	char channel_str[5];
	char *gpio_export_file = "/sys/class/gpio/export";

	int export_fd=open(gpio_export_file, O_WRONLY);
	if (export_fd < 0)
	{
		printf("Cannot open GPIO to export %d\n", p_dat->gpio_base_no);
		return -1;
	}

	sprintf(channel_str, "%d", p_dat->gpio_base_no + p_dat->gpio_pin_no);
	if(write(export_fd, channel_str, (strlen(channel_str)+1))<0)
	{
		printf("Cannot write GPIO to export %d\n", p_dat->gpio_base_no);
	}

	close(export_fd);
	return 0;
}

uint32_t gpio_close_channel(GPIO_PIN_DAT *p_dat)
{
	char channel_str[5];
	char *gpio_unexport_file = "/sys/class/gpio/unexport";

	int unexport_fd=open(gpio_unexport_file, O_WRONLY);
	if (unexport_fd < 0)
	{
		printf("Cannot close GPIO by writing unexport %d\n", p_dat->gpio_base_no);
		return -1;
	}

	sprintf(channel_str, "%d", p_dat->gpio_base_no+p_dat->gpio_pin_no);
	if(write(unexport_fd, channel_str, (strlen(channel_str)+1))<0)
	{
		printf("Cannot write GPIO to unexport %d\n", p_dat->gpio_base_no);
	}

	close(unexport_fd);
	return 0;
}

uint32_t gpio_setup( GPIO_PIN_DAT *p_dat)
{
    char path[PATH_MAX];
    char direction_str[5];

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/%s", p_dat->gpio_base_no+p_dat->gpio_pin_no, "direction");
    int fd=open(path, O_WRONLY);
    if(fd < 0)
    {
    	printf("Cannot open GPIO to direction %s\n", path);
    	return -1;
    }

    if(p_dat->gpio_set_dir == GPIO_DIR_IN)
    	strcpy(direction_str, "in");
    else if(p_dat->gpio_set_dir == GPIO_DIR_OUT)
    	strcpy(direction_str, "out");

    if(write(fd, direction_str, strlen(direction_str)+1) < 0)
	{
		printf("Cannot write GPIO to direction %s\n", direction_str);
		close(fd);
		return -1;
	}

    close(fd);

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/%s", p_dat->gpio_base_no+p_dat->gpio_pin_no, "value");
	p_dat->gpio_pin_fd=open(path, O_RDWR);
	if(p_dat->gpio_pin_fd < 0)
	{
		printf("Cannot open GPIO to value %s\n", path);
		return -1;
	}

    return 0;
}

void gpio_write( GPIO_PIN_DAT *p_dat)
{
    if(p_dat->gpio_value)
        write(p_dat->gpio_pin_fd, "1", 1);
    else
        write(p_dat->gpio_pin_fd, "0", 1);
}

int32_t gpio_read( GPIO_PIN_DAT *p_dat)
{
    char buff[2];

    lseek(p_dat->gpio_pin_fd,0, SEEK_SET);
    if(read(p_dat->gpio_pin_fd, buff, sizeof(buff)) > 0)
        return *buff=='1' ? 1 : 0;
    return -1;
}

void gpio_write_pin( GPIO_PIN_DAT *p_dat, uint32_t state)
{
    if(state)   // everything but LOW is HIGH
        write(p_dat->gpio_pin_fd, "1", 1);
    else
        write(p_dat->gpio_pin_fd, "0", 1);
}

void gpio_close( GPIO_PIN_DAT *p_dat)
{
    close(p_dat->gpio_pin_fd);
}
