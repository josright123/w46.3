/**
 *******************************************************************************
 * @file    at32f437_int7.c
 * @brief   Hardware Abstraction Layer of DM9051 Ethernet Controller
 * 
 * @details This file provides functions for SPI initialization and communication with
 *          the DM9051 Ethernet controller.
 * 
 * @version 1.6.1
 * @author  Joseph CHANG
 * @copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * @date    2025-02-27
 *
 * @api     MCU Irq Control:
 *	    - hal_irqline() : ext line no
 *	    - hal_disable_mcu_irq()  : (disable)
 *          - hal_enable_mcu_irq()  : enable
 *	    - hal_int_initialize() - davicom_halintrruptinit() : interrupt configuration
 *******************************************************************************
 */
#include "hal/hal_at32f437.h"

static char *int_info[] = {
	"AT32F437 INTERRUPT GPIO",
	"int/ pc7",
};

#define INTSCFG_CLK	 CRM_SCFG_PERIPH_CLOCK
#define INTPORT_CLK	 CRM_GPIOC_PERIPH_CLOCK

#define EXINT_SCFG	{SCFG_PORT_SOURCE_GPIOC, SCFG_PINS_SOURCE7};
#define EXINT_STRUCT	{EXINT9_5_IRQn, NVIC_PRIORITY_GROUP_0, EXINT_LINE_7, EXINT_TRIGGER_FALLING_EDGE,};

#define PIN_IRQ		{GPIOC, GPIO_PINS_7,} //pc7

struct scfg_t {
	scfg_port_source_type port_source;
	scfg_pins_source_type pin_source;
};

struct exint_t {
	IRQn_Type irqn;
	nvic_priority_group_type priority_group;
	uint32_t line;
	exint_polarity_config_type polarity;
};

/* Program Data
 */
const static struct scfg_t s1 = EXINT_SCFG;
const static struct exint_t es1 = EXINT_STRUCT;
const struct gpio_t p1 = PIN_IRQ;

//#include "trans/hal_trans.h"
//#include "hal/trans/hal_export.h" (But by hal/trans/hal_trans.h in dm9051.c)
//#include "hal/trans/hal_encode.h" (But by hal/trans/hal_trans.h in dm9051.c)

/* Hal - abstract for INT
 */
char *hal_int_info(int index)
{
	return int_info[index];
}

/**
 * @brief  Interrupt Initialization Function
 * @param  none
 */
uint32_t hal_int_initialize(void)
{
	gpio_init_type gpio_init_struct;
	exint_init_type exint_init_struct;

	crm_periph_clock_enable(INTPORT_CLK, TRUE); //pindata->clock
	crm_periph_clock_enable(INTSCFG_CLK, TRUE);

	/* Initialize GPIO structure with default values */
	gpio_default_para_init(&gpio_init_struct);

	/* Init GPIO */
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT; //int_gpio.mode;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init_struct.gpio_pins = p1.pin;
	gpio_init(p1.port, &gpio_init_struct);

	/* Configure EXINT */
	scfg_exint_line_config(s1.port_source, s1.pin_source);

	/* Init EXINT */
	exint_default_para_init(&exint_init_struct);
	exint_init_struct.line_enable = TRUE;
	exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
	exint_init_struct.line_select = es1.line;
	exint_init_struct.line_polarity = es1.polarity;
	exint_init(&exint_init_struct);
	return es1.line;
}

void hal_disable_mcu_irq(void){
	nvic_irq_disable(es1.irqn); // nvic_irqn()
}

void hal_enable_mcu_irq(void){
	nvic_priority_group_config(es1.priority_group); // nvic_prio()
	nvic_irq_enable(es1.irqn, 1, 0);
}

uint32_t hal_irqline(void){
	return es1.line;
}

int dm9051_interrupt_event = 0;

void hal_clr_int_event(uint32_t exint_line)
{
  if (exint_line == hal_irqline())
  {
    dm9051_interrupt_event = 0;
  }
}

void hal_set_int_event(uint32_t exint_line)
{
  if (exint_line == hal_irqline())
  {
    dm9051_interrupt_event = 1;
  }
}

int hat_get_int_event(uint32_t exint_line)
{
  if (exint_line == hal_irqline()) {
	  if (dm9051_interrupt_event) {
		  dm9051_interrupt_event = 0;
		  return 1;
	  }
  }
  return 0;
}

//.void EXINT9_5_IRQHandler(void) {
//.	if (exint_flag_get(hal_irqline()) != RESET)
//.	{
//.		hal_set_int_event(hal_irqline());
//.		exint_flag_clear(hal_irqline());
//.	}
//.}

/* definitly required, mandatory to be in this "at32f437_int7.c" */
/* EXINT9_5_IRQHandler is mcu orientation, function code is bind with f437's library code */
void EXINT9_5_IRQHandler(void) {
	if (exint_flag_get(hal_irqline()) != RESET)
	{
			hal_set_int_event(hal_irqline()); //[flag control in 'edriver']
			exint_flag_clear(hal_irqline());
	}
}
