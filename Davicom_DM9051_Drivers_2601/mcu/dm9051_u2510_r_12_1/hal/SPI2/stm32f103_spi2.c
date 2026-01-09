/**
 *******************************************************************************
 * @file    stm32f103_spi2.c
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
#include "hal/hal_stm32f103.h"
#include "core/dm9051.h"

static char *spi_info[] = {
	"STM32F103 ETHERNET SPI2",
	"sck/mo/mi/ pb13/pb15/pb14, cs/ pb12",
};

/* Hal - Implementation abstract for SPI
 */

/* Select SPI DM9051: Chip Select pin low  */
#define SPI_DM9051_CS_LOW()       GPIO_ResetBits(GPIOB, GPIO_Pin_12)
/* Deselect SPI DM9051: Chip Select pin high */
#define SPI_DM9051_CS_HIGH()      GPIO_SetBits(GPIOB, GPIO_Pin_12)

/* 复位 DM9051: 拉低 RSTB */
#define DM9051_RESET_LOW()   GPIO_ResetBits(GPIOD, GPIO_Pin_12)

/* 释放复位 DM9051: 拉高 RSTB */
#define DM9051_RESET_HIGH()  GPIO_SetBits(GPIOD, GPIO_Pin_12)

//...[hal_mcu_spi.c]............................................................................
void      ctick_delay_us(uint32_t nus);
void      ctick_delay_ms(uint16_t nms);

char *hal_spi_info(int index)
{
	return spi_info[index];
}

#define DM9051_HARDWARE_RESET()  do { \
    DM9051_RESET_LOW();  /* 拉低 RSTB 进入复位状态 */ \
    ctick_delay_us(10);        /* 保持 10?s，符合 DM9051 复位时序要求 */ \
    DM9051_RESET_HIGH(); /* 释放 RSTB 退出复位状态 */ \
    ctick_delay_ms(1);         /* 额外等待 1ms，确保 DM9051 启动完成 */ \
} while(0)

/**
 * @brief  SPI Initialization Function
 * @param  none
 */
void hal_spi_initialize(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable SPI1 and GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOF, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* Configure SPI2 pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // PB13, PB14, PB15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure I/O for SPI2 Chip select */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;		  // 使用 PB12 作为片选信号
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 高速模式
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 配置RSTB
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    // 复位完成后，默认拉高 `RSTB`
    GPIO_SetBits(GPIOD, GPIO_Pin_12);

	/* Configure SPI2 */
	// 配置数据传输方向为全双工模式（发送和接收使用独立的数据线）
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	// 配置 SPI 工作模式为主机模式
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	// 配置数据帧长度为 8 位
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	// 配置时钟相位为第一边沿采样数据
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	// 配置 NSS 信号管理方式为软件控制
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI2 */
	SPI_Cmd(SPI2, ENABLE);
}
		
/**
 * @brief  Sets GPIO pin to low state
 */
static __inline void hal_stdpin_lo(void){
	SPI_DM9051_CS_LOW();
}

/**
 * @brief  Sets GPIO pin to high state
 */
static __inline void hal_stdpin_hi(void){
	SPI_DM9051_CS_HIGH();
}

// Inner - SPI Transfer Function
static __inline u8 SPI_DM9051_SendByte(u8 byte)
{
    /* 等待 SPI TXE 标志，确保上一次数据传输完成 */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == 0);

    /* 发送数据到 SPI 总线 */
    SPI_I2S_SendData(SPI2, byte);

    /* 等待 SPI RXNE 标志，确保接收到数据 */
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == 0);

    /* 在读取 MISO 之前稍作延迟，避免 MISO 时序问题 */
    //Delay_us(1);  // 根据 T6 MISO 输出延迟（5~7ns），加上额外的安全裕量

    /* 读取 SPI 总线返回的数据 */
    return SPI_I2S_ReceiveData(SPI2);
}

// HAL - SPI Data Read Function
static __inline void hal_spi_data_read(uint8_t reg, uint8_t *pd)
{
	SPI_DM9051_SendByte(reg);
	*pd = SPI_DM9051_SendByte(0x0);
}

// HAL - SPI Data Write Function
static __inline void hal_spi_data_write(uint8_t reg, uint8_t val)
{
	SPI_DM9051_SendByte(reg);  // 发送寄存器地址
	SPI_DM9051_SendByte(val); // 发送数据
}

// HAL - Memory Read and Write Functions
static __inline void hal_spi_mem_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t i;
	SPI_DM9051_SendByte(reg);
	for (i = 0; i < len; i++)
	buf[i] = SPI_DM9051_SendByte(0x0);
}

static __inline void hal_spi_mem_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t i;
	SPI_DM9051_SendByte(reg); // | OPC_REG_W
	for (i = 0; i < len; i++)
	SPI_DM9051_SendByte(buf[i]);
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
