/**
 *******************************************************************************
 * @file    at32f403a_int7.c
 * @brief   Hardware Abstraction Layer for DM9051 Interrupt Handling on AT32F403A
 * 
 * @details This file provides functions for external interrupt initialization
 *          and control for the DM9051 Ethernet controller on AT32F403A platform.
 *          
 *          Pin Configuration:
 *          - DM9051_INT: PC7 (External Interrupt Line 7)
 * 
 * @version 1.6.1
 * @author  Joseph CHANG
 * @copyright (c) 2023-2026 Davicom Semiconductor, Inc.
 * @date    2025-06-29
 *
 * @api     MCU Interrupt Control:
 *          - hal_irqline(): Get external interrupt line number
 *          - hal_disable_mcu_irq(): Disable MCU interrupt
 *          - hal_enable_mcu_irq(): Enable MCU interrupt
 *          - hal_int_initialize(): Initialize interrupt configuration
 *******************************************************************************
 */
#include "hal/hal_at32f403a.h"

static char *int_info[] = {
	"AT32F403A INTERRUPT GPIO",
	"int/ pc7",
};

/* Clock Configuration for AT32F403A */
#define INTSCFG_CLK	    CRM_IOMUX_PERIPH_CLOCK     /* AT32F403A uses IOMUX instead of SCFG */
#define INTPORT_CLK	    CRM_GPIOC_PERIPH_CLOCK

/* External Interrupt Configuration for AT32F403A */
#define EXINT_IOMUX	    {GPIO_PORT_SOURCE_GPIOC, GPIO_PINS_SOURCE7};
#define EXINT_STRUCT	{EXINT9_5_IRQn, NVIC_PRIORITY_GROUP_4, EXINT_LINE_7, EXINT_TRIGGER_FALLING_EDGE,};

//#define PIN_IRQ		{GPIOC, GPIO_PINS_7,}
#define IRQ_PORT		GPIOC
#define IRQ_PIN			GPIO_PINS_7

/**
 * @brief IOMUX configuration structure for AT32F403A
 */
struct iomux_t {
	gpio_port_source_type port_source;
	gpio_pins_source_type pin_source;
};

/**
 * @brief External interrupt configuration structure
 */
struct exint_t {
	IRQn_Type irqn;
	nvic_priority_group_type priority_group;
	uint32_t line;
	exint_polarity_config_type polarity;
};

/* Static Configuration Data */
const static struct iomux_t s1 = EXINT_IOMUX;
const static struct exint_t es1 = EXINT_STRUCT;
//const struct gpio_t p7 = PIN_IRQ;

/**
 * @brief  Get interrupt information string
 * @param  index: Information index (0 or 1)
 * @retval Pointer to information string
 */
char *hal_int_info(int index)
{
	return int_info[index];
}

/**
 * @brief  Initialize external interrupt for DM9051
 * @note   Configures PC7 as external interrupt input for DM9051 INT signal
 * @param  none
 * @retval External interrupt line number
 */
uint32_t hal_int_initialize(void)
{
	gpio_init_type gpio_init_struct;
	exint_init_type exint_init_struct;
	
	printf("[DRIVER init] AT32F403A INT Running...\r\n");

	/* Enable peripheral clocks */
	crm_periph_clock_enable(INTPORT_CLK, TRUE);     /* Enable GPIOC clock */
	crm_periph_clock_enable(INTSCFG_CLK, TRUE);     /* Enable IOMUX clock */

	/* Initialize GPIO structure with default values */
	gpio_default_para_init(&gpio_init_struct);

	/* Configure PC7 as input for external interrupt */
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;      /* Pull-up for active-low interrupt */
	gpio_init_struct.gpio_pins = IRQ_PIN; //p7.pin
	gpio_init(IRQ_PORT, &gpio_init_struct); //p7.port

	/* Configure external interrupt line mapping */
	gpio_exint_line_config(s1.port_source, s1.pin_source);

	/* Initialize external interrupt configuration */
	exint_default_para_init(&exint_init_struct);
	exint_init_struct.line_enable = TRUE;
	exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
	exint_init_struct.line_select = es1.line;
	exint_init_struct.line_polarity = es1.polarity;  /* Falling edge for DM9051 */
	exint_init(&exint_init_struct);
	
	return es1.line;
}

/**
 * @brief  Disable MCU external interrupt
 * @note   Disables the NVIC interrupt for DM9051
 */
void hal_disable_mcu_irq(void)
{
	nvic_irq_disable(es1.irqn);
}

/**
 * @brief  Enable MCU external interrupt
 * @note   Configures NVIC priority and enables interrupt for DM9051
 */
void hal_enable_mcu_irq(void)
{
	nvic_priority_group_config(es1.priority_group);
	nvic_irq_enable(es1.irqn, 1, 0);
}

/**
 * @brief  Get external interrupt line number
 * @retval External interrupt line number
 */
uint32_t hal_irqline(void)
{
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

void EXINT9_5_IRQHandler(void) {
	if (exint_flag_get(hal_irqline()) != RESET)
	{
		hal_set_int_event(hal_irqline());
		exint_flag_clear(hal_irqline());
	}
}
