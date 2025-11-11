/**
 *******************************************************************************
 * @file    stm32f401_spi2.c
 * @brief   Hardware Abstraction Layer for DM9051 Ethernet Controller
 * 
 * @details This file provides functions for SPI initialization and communication with
 *          the DM9051 Ethernet controller.
 * 
 * @version 1.6
 * @author  Joseph CHANG
 * @copyright (c) 2023-2026 Davicom Semiconductor, Inc.
 * @date    2025-02-27
 *******************************************************************************
 */
#include "hal/hal_stm32f401.h"
#include "core/dm9051.h"

static char *int_info[] = {
	"STM32F401 ETHERNET SPI2",
	"sck/mo/mi/ pb13/pb15/pb14, cs/ pb12",
};

extern SPI_HandleTypeDef	 hspi2;
#define CAN_CS2_GPIO_Port	 GPIOB
#define CAN_CS2_Pin		 GPIO_PIN_12
#define SPI_TIMEOUT		 10

/* Hal - Implementation abstract for SPI
 */

char *hal_spi_info(int index)
{
	return spi_info[index];
}

/**
 * @brief  SPI2 Initialization Function
 * @param  none
 */
void hal_spi_initialize(void)
{
	//HWF_InitSPI2();=
	GPIO_InitTypeDef GPIO_InitStruct;

	printf("[%s mode] /%s %s\r\n", RX_MODE_STR, MCU_SPI_STR, SPI_PIN_STR);

	/* Peripheral clock enable */
	__SPI2_CLK_ENABLE();

	__GPIOB_CLK_ENABLE();
	/**SPI2 GPIO Configuration    
	PB13     ------> SPI2_SCK
	PB14     ------> SPI2_MISO
	PB15     ------> SPI2_MOSI 
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, CAN_CS2_Pin, GPIO_PIN_SET);
	/*Configure GPIO pin : CAN_CS_Pin */
	GPIO_InitStruct.Pin = CAN_CS2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(CAN_CS2_GPIO_Port, &GPIO_InitStruct);
		
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	hspi2.Init.CRCPolynomial = 10;
	//SPI_Init(SPI2, &SPI_HandleTypeDef);
	if (HAL_SPI_Init(&hspi2) != HAL_OK)
	{
	//Error_Handler();
	}
}

/**
 * @brief  Sets GPIO pin to low state
 * @param  gpio: Pointer to GPIO configuration structure
 */
static __inline void hal_stdpin_lo(void){
	HAL_GPIO_WritePin(CAN_CS2_GPIO_Port, CAN_CS2_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  Sets GPIO pin to high state
 * @param  gpio: Pointer to GPIO configuration structure
 */
static __inline void hal_stdpin_hi(void){
	HAL_GPIO_WritePin(CAN_CS2_GPIO_Port, CAN_CS2_Pin, GPIO_PIN_SET);
}

// Inner - SPI Data Read Function
static __inline void hal_spi_data_read(uint8_t reg, uint8_t *pd)
{
	uint8_t cmdaddr = reg;
	HAL_SPI_Transmit(&hspi2, &cmdaddr, 1, SPI_TIMEOUT);
	HAL_SPI_Receive(&hspi2, pd, 1, SPI_TIMEOUT);
}

// Inner - SPI Data Write Function
static __inline void hal_spi_data_write(uint8_t reg, uint8_t val)
{
	uint8_t cmdaddr = reg;
	uint8_t data = val;
	HAL_SPI_Transmit(&hspi2, &cmdaddr, 1, SPI_TIMEOUT);
	HAL_SPI_Transmit(&hspi2, &data, 1, SPI_TIMEOUT);
}

static __inline void hal_spi_mem_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint8_t cmdaddr = reg;
        HAL_SPI_Transmit(&hspi2, &cmdaddr, 1, SPI_TIMEOUT);
        HAL_SPI_Receive(&hspi2, buf, len, SPI_TIMEOUT*5);
}

static __inline void hal_spi_mem_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint8_t cmdaddr = reg;
	HAL_SPI_Transmit(&hspi2, &cmdaddr, 1, SPI_TIMEOUT);
	HAL_SPI_Transmit(&hspi2, buf, len, SPI_TIMEOUT*5);
}

/* Hardware Interface Functions */
uint8_t   hal_read_reg(uint8_t reg)
{
    uint8_t val;
    hal_stdpin_lo();
    hal_spi_data_read(reg | OPC_REG_R, &val);
    hal_stdpin_hi();
    return val;
}
void      hal_write_reg(uint8_t reg, uint8_t val)
{
    hal_stdpin_lo();
    hal_spi_data_write(reg | OPC_REG_W, val);
    hal_stdpin_hi();
}
void      hal_read_mem(uint8_t *buf, uint16_t len)
{
    hal_stdpin_lo();
    hal_spi_mem_read(DM9051_MRCMD | OPC_REG_R, buf, len);
    hal_stdpin_hi();
}
void      hal_write_mem(uint8_t *buf, uint16_t len)
{
    hal_stdpin_lo();
    hal_spi_mem_write(DM9051_MWCMD | OPC_REG_W, buf, len);
    hal_stdpin_hi();
}
