/**
 *******************************************************************************
 * @file    stm32f103_int7.c
 * @brief   Hardware Abstraction Layer of DM9051 Ethernet Controller
 * 
 * @details This file provides functions for SPI initialization and communication with
 *          the DM9051 Ethernet controller.
 * 
 * @version 1.6
 * @author  Joseph CHANG
 * @copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * @date    2025-02-27
 *
 * @api     MCU Irq Control:
 *	    - hal_irqline() : ext line no
 *	    - hal_disable_mcu_irq()  : (disable)
 *          - hal_enable_mcu_irq()  : enable
 *	    - davicom_halintrruptinit() : interrupt configuration
 *******************************************************************************
 */
#include "hal/hal_stm32f103.h"

static char *int_info[] = {
	"STM32F103 INTERRUPT GPIO",
	"int/ pc7",
};

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
	/* configure PC7 as external interrupt */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Connect DM9051 EXTI Line to GPIOC Pin 7 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);

	/* Configure DM9051 EXTI Line to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Clear DM9051 EXTI line pending bit */
	EXTI_ClearITPendingBit(EXTI_Line7);

	/* Enable the EXTI7 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	return EXTI_Line7;
}

void hal_disable_mcu_irq(void){
	//nvic_irq_disable(es2.irqn); // nvic_irqn()
}

void hal_enable_mcu_irq(void){
	//nvic_priority_group_config(es2.priority_group); // nvic_prio()
	//nvic_irq_enable(es2.irqn, 1, 0);
}

uint32_t hal_irqline(void){
	return EXTI_Line7;
}
