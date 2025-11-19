/**
  **************************************************************************
  * @file     main_env.c ,dm9051_env.c ,dm9051_config.c ,at32_emac.c
  * @version  v1.0
  * @date     2023-04-28
  * @brief    dm9051 config program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f415_board.h"
#include "developer_conf.h"
//#include "ethernetif.h"
//#include "dm9051_env.h" //("dm9051f_netconf.h")(contain "dm9051_lw.h")

//int flag_link = 0;
//extern int flag_link ;

/**
  * @brief  configures required pins.
  * @param  none
  * @retval none
  */
//static void dm9051_pins_configuration(void)
//{
//  //GPIO_InitType GPIO_InitStructure; (at32f4xx_gpio.h)
//  //gpio_init_type gpio_initstructure; (at32f415_gpio.h)
//  gpio_init_type gpio_initstructure;
//  spi_init_type spi_init_struct;
//	
//  //.spi_enable(SPI2, FALSE);
//  /* spi2 sck pin */
//  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
//  gpio_default_para_init(&gpio_initstructure);
//  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
//  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
//	
//  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
//  gpio_initstructure.gpio_mode         = GPIO_MODE_MUX; //= GPIO_MODE_OUTPUT;
//  gpio_initstructure.gpio_pins           = GPIO_PINS_13;
//  gpio_init(GPIOB, &gpio_initstructure);
//	
//  /* spi2 miso pin */
//  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
//  gpio_initstructure.gpio_mode         = GPIO_MODE_INPUT;
//  gpio_initstructure.gpio_pins           = GPIO_PINS_14;
//  gpio_init(GPIOB, &gpio_initstructure);
//  
//  /* spi2 mosi pin */
//  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
//  gpio_initstructure.gpio_mode         = GPIO_MODE_MUX; //= GPIO_MODE_OUTPUT;
//  gpio_initstructure.gpio_pins           = GPIO_PINS_15;
//  gpio_init(GPIOB, &gpio_initstructure);

//  /* spi_cs */
//  gpio_initstructure.gpio_pull           = GPIO_PULL_NONE;
//  gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
//  gpio_initstructure.gpio_pins           = GPIO_PINS_12;
//  gpio_init(GPIOB, &gpio_initstructure);
//  
//  /* xxx spi-config xxx */
//  crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE);
//  spi_default_para_init(&spi_init_struct);
//  
//  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
//  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
//  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_8;
//  //spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_LSB;
//	spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
//  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
//  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
//  //spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
//	spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
//  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
//  spi_init(SPI2, &spi_init_struct);
//  
//  spi_enable(SPI2, TRUE);
//}
			
/**
  * @brief  initialize !tmr6 --> tmr5 for emac
  * @param  none
  * @retval none
  */
static void dm9051_tmr_init(void)
{
  crm_clocks_freq_type crm_clocks_freq_struct = {0};
  crm_periph_clock_enable(CRM_TMR5_PERIPH_CLOCK, TRUE); //CRM_TMR6_PERIPH_CLOCK

  crm_clocks_freq_get(&crm_clocks_freq_struct);
  
  /* time base configuration */
  /* systemclock/24000/100 = 100hz */
  tmr_base_init(TMR5, 99, (crm_clocks_freq_struct.ahb_freq / 10000) - 1); //TMR6
  tmr_cnt_dir_set(TMR5, TMR_COUNT_UP); //TMR6

  /* overflow interrupt enable */
  tmr_interrupt_enable(TMR5, TMR_OVF_INT, TRUE); //TMR6

  /* tmr1 overflow interrupt nvic init */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(TMR5_GLOBAL_IRQn, 0, 0); //TMR6_GLOBAL_IRQn
  tmr_counter_enable(TMR5, TRUE); //TMR6
}

/**
  * @brief  enable /emac clock and /gpio clock
  * @retval success or error
  *
  * Called by main() / "main.c"
  *
  */
error_status env_main_system_init(void)
{
  //dm9051_pins_configuration();
  dm9051_tmr_init();
  return SUCCESS;
}
