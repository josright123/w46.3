/**
 **************************************************************************
 * @file    hal_gpio.c
 * @brief   GPIO Hardware Abstraction Layer implementation for AT32F437
 *
 * @details This module provides hardware abstraction for General Purpose
 *          Input/Output (GPIO) operations on the AT32F437 microcontroller.
 *          It includes pin configuration, multiplexing, and control operations.
 *
 * @note    Supported Features:
 *          - Standard and multiplexed pin configurations
 *          - Input/Output operations with configurable pull-up/down
 *          - Pin state control and reading
 *
 * @version 1.0
 * @author  Joseph CHANG
 * @copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * @date    2024-11-05
 *
 * @api     Pin Configuration:
 *          - AT_hal_stdpin_config()  : Standard pin configuration
 *           (AT_hal_muxpin_config()  : Multiplexed pin configuration)
 *
 *          Pin Control:
 *          - AT_hal_stdpin_lo()      : Set pin high
 *          - AT_hal_stdpin_hi()      : Set pin low
 *          - AT_hal_stdpin_get()     : Read pin state
 **************************************************************************
 */

#include "control/drv_control/conf_core.h"

#if defined(_DLW_AT32F437xx)

/*-----------------------------------------------------------------------------
 * GPIO Pin Configurations and Control Functions
 *
 * Example Implementations
 * The file includes two concrete GPIO implementations:
 * LED3 Pin
 *  .Output pin (PD14)
 *  .No pull-up/down
 *  .Functions: config_led3(), led3_lo(), led3_hi()
 * BUTTON Pin
 *  .Input pin (PA0)
 *  .Pull-down enabled
 *  .Functions: config_button(), button_is_pressed()
 * DIAG Pin
 *  .Output pin (PA15)
 *  .No pull-up/down
 *  .Functions: config_diag(), diag_lo(), diag_hi()
 * INPT Pin
 *  .Input pin (PC7)
 *  .Pull-up enabled
 *  .Functions: config_inpt(), inpt_get()
 *-----------------------------------------------------------------------------*/

/**
 * @brief GPIO configuration structures for commonly used pins
 */
struct gpio_config_t
	led3 = {
		GPIOD,		  // LED3_GPIO
		GPIO_PINS_14, // LED3_PIN
		CRM_GPIOD_PERIPH_CLOCK, // LED3_GPIO_CRM_CLK
		{
			GPIO_PULL_NONE,
			GPIO_MODE_OUTPUT,
		}
},
	button = {
		GPIOA,		 // USER_BUTTON_PORT
		GPIO_PINS_0, // USER_BUTTON_PIN
		CRM_GPIOA_PERIPH_CLOCK, // USER_BUTTON_CRM_CLK
		{
			GPIO_PULL_DOWN,
			GPIO_MODE_INPUT,
		}
},
	diag = {
		GPIOA,
		GPIO_PINS_15,
		CRM_GPIOA_PERIPH_CLOCK,
		{
			GPIO_PULL_NONE,
			GPIO_MODE_OUTPUT,
		}
},
	inpt = {
		GPIOC,
		GPIO_PINS_7,
		CRM_GPIOC_PERIPH_CLOCK,
		{
			GPIO_PULL_UP,
			GPIO_MODE_INPUT,
		}
};

/**
 * @brief  LED3 control functions
 */
void config_led3(void)
{
	dm9051if_stdpin_config(&led3);
}

void led3_toggle(void)
{
	led3.port->odt ^= led3.pin; //gpio_stdpin_toggle
}

void led3_on(void)
{
	dm9051if_stdpin_lo(&led3); //gpio_stdpin_lo
}

void led3_off(void)
{
	dm9051if_stdpin_hi(&led3); //gpio_stdpin_hi
}

/**
 * @brief  User button control functions
 */
void config_button(void)
{
	dm9051if_stdpin_config(&button);
}

button_type button_is_pressed(void)
{
	return (gpio_stdpin_get(&button) == SET) ? USER_BUTTON : NO_BUTTON;
}

/**
 * @brief  Periodically to Demo toggle led3
 */
#define NMS 250

struct led_control_t {
    uint32_t start_time;
    uint32_t interval;
    confirm_state is_active; //bool is_active;
    //sled_ops_state state;
};

static struct led_control_t led_control[2] = {0};

void led_start_alloc(trigger_type trigger, uint32_t now)
{
	struct led_control_t *ctrl = &led_control[trigger];

	ctrl->start_time = now;

	trigger++;
	if (trigger > VIA_NET)
		trigger = VIA_BUTTON;
	ctrl = &led_control[trigger];
	ctrl->start_time = now + (NMS >> 1);
}

void operate_led3(trigger_type trigger, led_ops_state ops)
{
	//static uint32_t statime[2] = {0, 0};
	//static uint32_t intvltime[2];
	struct led_control_t *ctrl = &led_control[trigger];
	uint32_t elapsed;

	if (ops == LED_FLASH)
	{
		uint32_t now = dm9051_boards_heartbeat_now();
		if (!ctrl->is_active) //(!statime[trigger])
		{
			led_start_alloc(trigger, now); //ctrl->start_time = now; //statime[trigger] = now;
			ctrl->interval = NMS; //intvltime[trigger] = NMS;
			ctrl->is_active = TRUE;
			led3_toggle(); //led3_on
			return;
		}

		elapsed = now - ctrl->start_time; //statime[trigger];
		if (elapsed >= ctrl->interval) //intvltime[trigger]
		{
			if (ctrl->interval == NMS) //intvltime[trigger]
				led3_toggle(); //led3_off
			if (ctrl->interval == (NMS * 2))
				ctrl->is_active = FALSE; //statime[trigger] = 0;
			ctrl->interval += NMS;
		}
	}
	else
	{
		/* for only turn led off once by per operated trigger source!
		 */
		if (ctrl->is_active) { //statime[trigger]
			led3_off();
			ctrl->is_active = FALSE; //statime[trigger] = 0;
		}
	}
}

/**
 * @brief  Diagnostic pin control functions
 */
void config_diag(void)
{
	dm9051if_stdpin_config(&diag);
}

void diag_lo(void)
{
	dm9051if_stdpin_lo(&diag); //gpio_stdpin_lo
}

void diag_hi(void)
{
	dm9051if_stdpin_hi(&diag); //gpio_stdpin_hi
}

/**
 * @brief  Input pin control functions
 */
void config_inpt(void)
{
	dm9051if_stdpin_config(&inpt);
}

flag_status inpt_get(void)
{
	return gpio_stdpin_get(&inpt);
}

#endif /* _DLW_AT32F437xx */

// Configure and use LED3
//config_led3();
//led3_toggle();
//led3_on();
//led3_off();

//_config_button();
//_button_is_pressed();
