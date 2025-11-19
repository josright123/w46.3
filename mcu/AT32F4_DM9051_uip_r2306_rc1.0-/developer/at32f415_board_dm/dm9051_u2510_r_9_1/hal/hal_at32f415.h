/**
 *******************************************************************************
 * @file    hal_at32f415.h
 * @brief   Hardware Abstraction Layer header for AT32F403A platform
 * 
 * @details This file provides platform-specific definitions for the AT32F403A
 *          microcontroller used with DM9051 Ethernet controller.
 * 
 * @version 1.6.1
 * @author  Joseph CHANG
 * @copyright (c) 2023-2026 Davicom Semiconductor, Inc.
 * @date    2025-06-29
 *******************************************************************************
 */
#ifndef __HAL_AT32F415_H
#define __HAL_AT32F415_H

#include "at32f415_board.h"    /* MCU board support package */
#include "at32f415_clock.h"    /* MCU clock configuration */

/**
 * @brief GPIO port and pin structure for AT32F403A
 */
struct gpio_t {
	gpio_type *port;
	uint16_t pin;
};

#endif /* __HAL_AT32F415_H */
