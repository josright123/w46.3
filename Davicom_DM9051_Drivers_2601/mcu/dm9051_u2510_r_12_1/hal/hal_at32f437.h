#ifndef __HAL_AT32F437_H
#define __HAL_AT32F437_H

#include "at32f435_437_board.h" //mcu's board
#include "at32f435_437_clock.h" //Also mcu's clock

struct gpio_t {
	gpio_type *port;
	uint16_t pin;
};

#endif //__HAL_AT32F437_H
