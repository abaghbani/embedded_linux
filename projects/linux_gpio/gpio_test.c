/*
 * gpio_test.c
 *
 *  Created on: Sep 22, 2021
 *      Author: Akbar
 */


#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "gpio.h"

int main()
{
	GPIO_PIN_DAT gpio_pin;

    gpio_pin.gpio_base_no=906;
    gpio_pin.gpio_pin_no = 7;
    gpio_pin.gpio_set_dir=GPIO_DIR_OUT;
    gpio_open_channel(&gpio_pin);

    if(gpio_setup(&gpio_pin) < 0)
    {
    	printf("GPIO: setup is failed.\n");
    }
    else
    	printf("GPIO: setup is done successfully.\n");

    // write 0 to this pin
	gpio_write_pin(&gpio_pin, 0);
	// write 1 to this pin
	gpio_write_pin(&gpio_pin, 1);

    gpio_close(&gpio_pin);
    gpio_close_channel(&gpio_pin);

    return 0;
}
