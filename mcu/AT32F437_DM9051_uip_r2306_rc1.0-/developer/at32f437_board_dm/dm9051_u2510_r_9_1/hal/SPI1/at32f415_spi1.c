/**
 *******************************************************************************
 * @file    at32f403a_spi1.c
 * @brief   Hardware Abstraction Layer for DM9051 Ethernet Controller on AT32F403A
 * 
 * @details This file provides functions for SPI initialization and communication with
 *          the DM9051 Ethernet controller on AT32F403A platform.
 *          
 *          Pin Configuration:
 *          - SPI1_SCK:  PA5
 *          - SPI1_MOSI: PA7  
 *          - SPI1_MISO: PA6
 *          - SPI1_CS:   PA15 (GPIO controlled)
 * 
 * @version 1.6.1
 * @author  Joseph CHANG
 * @copyright (c) 2023-2026 Davicom Semiconductor, Inc.
 * @date    2025-06-29
 *******************************************************************************
 */
#include "hal/hal_at32f415.h"
//#include "core/dm9051.h"
#include "core/dm9051_constants.h"

static char *spi_info[] = {
	"AT32F403A ETHERNET SPI1 PROGRAM I/O MODE",
	"sck/mosi/miso/ pa5/pa7/pa6, cs/ pb12",
};

/* SPI and GPIO Configuration Macros for AT32F403A */
#define SPIDATA_CLK	    CRM_SPI1_PERIPH_CLOCK
#define csGPIOPORT_CLK	CRM_GPIOB_PERIPH_CLOCK
#define GPIOPORT_CLK	CRM_GPIOA_PERIPH_CLOCK

#define SPIDATA_SPI	    SPI1
#define PINS_CLKMOMI	{GPIOA, GPIO_PINS_5 | GPIO_PINS_7 | GPIO_PINS_6,}

/* AT32F403A uses different MUX configuration compared to F437 */
//#define MUX_CK		    { GPIO_PINS_SOURCE5, GPIO_MUX_5,}
//#define MUX_MO		    { GPIO_PINS_SOURCE7, GPIO_MUX_5,}
//#define MUX_MI		    { GPIO_PINS_SOURCE6, GPIO_MUX_5,}

#define CSPORT		    GPIOB
#define CSPIN		    GPIO_PINS_12

/**
 * @brief GPIO multiplexer configuration structure
 */
//struct multiplex_t {
//	gpio_pins_source_type source;
//	gpio_mux_sel_type mux;
//};

/* Static Configuration Data */
const static struct gpio_t p = PINS_CLKMOMI;
//const static struct multiplex_t m[] = {MUX_CK, MUX_MO, MUX_MI};

/**
 * @brief  Get SPI information string
 * @param  index: Information index (0 or 1)
 * @retval Pointer to information string
 */
char *hal_spi_info(int index)
{
	crm_clocks_freq_type crm_clocks_freq_struct = {0};
	crm_clocks_freq_get(&crm_clocks_freq_struct);		

	printf("[DRIVER %s mode] CLK:%d(sclk_freq) %d %d(apb2_freq) %d(apb1_freq) \r\n",
		hal_active_interrupt_desc(),
		crm_clocks_freq_struct.sclk_freq,
		crm_clocks_freq_struct.ahb_freq,
		crm_clocks_freq_struct.apb2_freq,
		crm_clocks_freq_struct.apb1_freq);
	printf("[DRIVER %s mode] SPI CLK use %s %dMhz, set to %dMhz\r\n", 
		hal_active_interrupt_desc(), "apb2_freq", 
		(crm_clocks_freq_struct.apb2_freq / 1000000),
		(crm_clocks_freq_struct.apb2_freq / 1000000) / (2 << SPIDATA_SPI->ctrl1_bit.mdiv_l)); //
	return spi_info[index];
}

/**
 * @brief  Initialize SPI1 peripheral for DM9051 communication
 * @note   Configures SPI1 in master mode with 8-bit data frame
 * @param  none
 * @retval none
 */
static spi_init_type spi_init_struct; // Make it static global

void hal_board_spi_info(void )
{
	crm_clocks_freq_type crm_clocks_freq_struct = {0};
	crm_clocks_freq_get(&crm_clocks_freq_struct);

	printf("[hal] at32f437_spi1 'spi' Running %dMhz...\r\n",
		(crm_clocks_freq_struct.apb2_freq / 1000000) / (2 << spi_init_struct.mclk_freq_division));
}

void hal_spi_speed_info(void)
{
	crm_clocks_freq_type crm_clocks_freq_struct = {0};
	crm_clocks_freq_get(&crm_clocks_freq_struct);

//	printf("[CLK]\r\n %d(sclk_freq) %d\r\n",
//		crm_clocks_freq_struct.sclk_freq,
//		crm_clocks_freq_struct.ahb_freq);
//	printf(" %d(apb2_freq) %d(apb1_freq) \r\n",
//		crm_clocks_freq_struct.apb2_freq,
//		crm_clocks_freq_struct.apb1_freq);

	printf(" (%s) %dMhz, set SPI CLK %dMhz\r\n", 
		"apb2_freq", 
		(crm_clocks_freq_struct.apb2_freq / 1000000),
		(crm_clocks_freq_struct.apb2_freq / 1000000) / (2 << spi_init_struct.mclk_freq_division));
}

void hal_spi_initialize(void)
{
	//spi_init_type spi_init_struct;
	gpio_init_type gpio_init_struct;
	
//	printf("[DRIVER init] AT32F403A SPI1 Running...\r\n");

	crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE); //Non-f437,iomux-config
	gpio_pin_remap_config(SWJTAG_GMUX_010, TRUE); //Non-f437,iomux-config

	/* Enable SPI1 peripheral clock */
	crm_periph_clock_enable(SPIDATA_CLK, TRUE);
	
	/* Initialize SPI configuration structure */
	spi_default_para_init(&spi_init_struct);
	
	/* Configure SPI1 parameters */
	spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
	spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
	spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_2; //SPI_MCLK_DIV_4; //SPI_MCLK_DIV_2;        /* SPI clock = PCLK/2 */
	spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
	spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
	spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;    /* CPOL = 0 */
	spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;        /* CPHA = 0 */

	/* Initialize and enable SPI1 */
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
	spi_init(SPIDATA_SPI, &spi_init_struct);
	spi_enable(SPIDATA_SPI, TRUE);

	/* Enable GPIOA peripheral clock */
	crm_periph_clock_enable(csGPIOPORT_CLK, TRUE);
	crm_periph_clock_enable(GPIOPORT_CLK, TRUE);

	/* Configure CS (Chip Select) GPIO as output */
	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init_struct.gpio_pins = CSPIN;
	gpio_init(CSPORT, &gpio_init_struct);
	
	/* Set CS high (inactive) initially */
	gpio_bits_set(CSPORT, CSPIN);

	/* Configure SPI pins (SCK, MOSI, MISO) as alternate function */
	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init_struct.gpio_pins = p.pin;
	gpio_init(p.port, &gpio_init_struct);

	/* Configure alternate function multiplexer for SPI pins */
//	gpio_pin_mux_config(p.port, m[0].source, m[0].mux);  /* SCK */
//	gpio_pin_mux_config(p.port, m[1].source, m[1].mux);  /* MOSI */
//	gpio_pin_mux_config(p.port, m[2].source, m[2].mux);  /* MISO */
	printf("[DRIVER %s mode] %s\r\n",
		hal_active_interrupt_desc(),
		spi_info[0]);
	printf("[DRIVER %s mode] %s\r\n",
		hal_active_interrupt_desc(),
		spi_info[1]);
}

/**
 * @brief  Set CS pin to low state (active)
 * @note   Inline function for performance
 */
static __inline void hal_stdpin_lo(void)
{
	gpio_bits_reset(CSPORT, CSPIN);
}

/**
 * @brief  Set CS pin to high state (inactive)
 * @note   Inline function for performance
 */
static __inline void hal_stdpin_hi(void)
{
	gpio_bits_set(CSPORT, CSPIN);
}

/**
 * @brief  SPI write transfer with simultaneous read
 * @param  byte: Data byte to transmit
 * @retval Received byte from SPI
 */
#define hal_spi_cmdxfer hal_spi_writexfer
static __inline uint8_t hal_spi_writexfer(uint8_t byte)
{
	/* Wait for transmit buffer empty */
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_TDBE_FLAG) == RESET);
	
	/* Transmit data */
	spi_i2s_data_transmit(SPIDATA_SPI, byte);
	
	/* Wait for receive buffer full */
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_RDBF_FLAG) == RESET);
	
	/* Return received data */
	return (uint8_t)spi_i2s_data_receive(SPIDATA_SPI);
}

/**
 * @brief  SPI read transfer (transmit dummy byte)
 * @retval Received byte from SPI
 */
static __inline uint8_t hal_spi_readxfer(void)
{
	/* Wait for transmit buffer empty */
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_TDBE_FLAG) == RESET);
	
	/* Transmit dummy byte */
	spi_i2s_data_transmit(SPIDATA_SPI, 0);
	
	/* Wait for receive buffer full */
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_RDBF_FLAG) == RESET);
	
	/* Return received data */
	return (uint8_t)spi_i2s_data_receive(SPIDATA_SPI);
}

/**
 * @brief  Read single register from DM9051
 * @param  reg: Register address
 * @param  pd: Pointer to store read data
 */
static __inline void hal_spi_data_read(uint8_t reg, uint8_t *pd)
{
	hal_spi_cmdxfer(reg);
	*pd = hal_spi_readxfer();
}

/**
 * @brief  Write single register to DM9051
 * @param  reg: Register address
 * @param  val: Data value to write
 */
static __inline void hal_spi_data_write(uint8_t reg, uint8_t val)
{
	hal_spi_cmdxfer(reg);
	hal_spi_writexfer(val);
}

/**
 * @brief  Read multiple bytes from DM9051 memory
 * @param  reg: Register address
 * @param  buf: Buffer to store read data
 * @param  len: Number of bytes to read
 */
static __inline void hal_spi_mem_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t i;
	hal_spi_cmdxfer(reg);
	for (i = 0; i < len; i++) {
		buf[i] = hal_spi_readxfer();
	}
}

/**
 * @brief  Write multiple bytes to DM9051 memory
 * @param  reg: Register address
 * @param  buf: Buffer containing data to write
 * @param  len: Number of bytes to write
 */
static __inline void hal_spi_mem_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t i;
	hal_spi_cmdxfer(reg);
	for (i = 0; i < len; i++) {
		hal_spi_writexfer(buf[i]);
	}
}

/* Hardware Interface Functions - Public API */

/**
 * @brief  Read single register from DM9051
 * @param  reg: Register address
 * @retval Register value
 */
uint8_t hal_read_reg(uint8_t reg)
{
	uint8_t val;
	hal_stdpin_lo();
	hal_spi_data_read(reg | OPC_REG_R, &val);
	hal_stdpin_hi();
	return val;
}

/**
 * @brief  Write single register to DM9051
 * @param  reg: Register address
 * @param  val: Value to write
 */
void hal_write_reg(uint8_t reg, uint8_t val)
{
	hal_stdpin_lo();
	hal_spi_data_write(reg | OPC_REG_W, val);
	hal_stdpin_hi();
}

/**
 * @brief  Read data from DM9051 memory
 * @param  buf: Buffer to store read data
 * @param  len: Number of bytes to read
 */
void hal_read_mem(uint8_t *buf, uint16_t len)
{
	hal_stdpin_lo();
	hal_spi_mem_read(DM9051_MRCMD | OPC_REG_R, buf, len);
	hal_stdpin_hi();
}

void hal_stop_mem(void)
{
	hal_stdpin_lo();
	hal_spi_cmdxfer(DM9051_VIDL | OPC_REG_R); //.
	hal_stdpin_hi();
}

/**
 * @brief  Write data to DM9051 memory
 * @param  buf: Buffer containing data to write
 * @param  len: Number of bytes to write
 */
void hal_write_mem(uint8_t *buf, uint16_t len)
{
	hal_stdpin_lo();
	hal_spi_mem_write(DM9051_MWCMD | OPC_REG_W, buf, len);
	hal_stdpin_hi();
}
