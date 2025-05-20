/*
 * gpio.h
 *
 *  Created on: Sep 22, 2021
 *      Author: Akbar
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define GPIO_DIR_OUT 1
#define GPIO_DIR_IN  0

typedef struct
{
    uint32_t    gpio_value;
    uint32_t	gpio_base_no;
    uint32_t    gpio_pin_no;
    int         gpio_pin_fd;
    uint32_t    gpio_set_dir;
} GPIO_PIN_DAT;

uint32_t gpio_open_channel(GPIO_PIN_DAT *);
uint32_t gpio_close_channel(GPIO_PIN_DAT *);
uint32_t gpio_setup( GPIO_PIN_DAT *);
void gpio_write( GPIO_PIN_DAT *);
int32_t gpio_read( GPIO_PIN_DAT *);
void gpio_write_pin( GPIO_PIN_DAT *, uint32_t);
void gpio_close( GPIO_PIN_DAT *);

#endif
