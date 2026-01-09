/**
 *******************************************************************************
 * @file    at32f437_spi1.c
 * @brief   Hardware Abstraction Layer for DM9051 Ethernet Controller
 * 
 * @details This file provides functions for SPI initialization and communication with
 *          the DM9051 Ethernet controller.
 * 
 * @version 1.6.1
 * @author  Joseph CHANG
 * @copyright (c) 2023-2026 Davicom Semiconductor, Inc.
 * @date    2025-02-27
 *******************************************************************************
 */
#include "hal/hal_at32f437.h"
#include "core/dm9051_constants.h"

static char *spi_info[] = {
	"AT32F437 ETHERNET SPI1",
	"sck/mo/mi/ pa5/pa7/pa6, cs/ pa15",
};

#define SPIDATA_CLK	CRM_SPI1_PERIPH_CLOCK
#define GPIOPORT_CLK	CRM_GPIOA_PERIPH_CLOCK

#define SPIDATA_SPI	SPI1
#define PINS_CLKMOMI	{GPIOA, GPIO_PINS_5 | GPIO_PINS_7 | GPIO_PINS_6,}

#define MUX_CK		{ GPIO_PINS_SOURCE5, GPIO_MUX_5,}
#define MUX_MO		{ GPIO_PINS_SOURCE7, GPIO_MUX_5,}
#define MUX_MI		{ GPIO_PINS_SOURCE6, GPIO_MUX_5,}

#define CSPORT		GPIOA
#define CSPIN		GPIO_PINS_15

struct multiplex_t {
	gpio_pins_source_type source;
	gpio_mux_sel_type mux;
};

/* Program Data
 */
const static struct gpio_t p = PINS_CLKMOMI;
const static struct multiplex_t m[] = {MUX_CK, MUX_MO, MUX_MI};

/* Hal - Implementation abstract for SPI
 */
char *hal_spi_info(int index)
{
	return spi_info[index];
}

/**
 * @brief  SPI Initialization Function
 * @param  none
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
	gpio_init_type gpio_init_struct;

	// Enable peripheral clock and initialize SPI
	crm_periph_clock_enable(SPIDATA_CLK, TRUE); //spi->clock
	spi_default_para_init(&spi_init_struct);

	spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
	spi_init_struct.master_slave_mode = SPI_MODE_MASTER;

	spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_4; //to be used, for 125/4 = 31MHz
	spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_8; //now, 125/8 = 15MHz

	spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
	spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
	spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
	spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;

	spi_init(SPIDATA_SPI, &spi_init_struct); //SPI1, spi->spi
	spi_enable(SPIDATA_SPI, TRUE); //SPI1

	/* Enable peripheral clock and initialize gpios */
	crm_periph_clock_enable(GPIOPORT_CLK, TRUE); //pindata->clock

	/* Configure cs GPIO */
	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT; //p.mode;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init_struct.gpio_pins = CSPIN;
	gpio_init(CSPORT, &gpio_init_struct);

	/* Configure clk/mo/mi GPIOs */
	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = GPIO_MODE_MUX; //p.mode;
	gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
	gpio_init_struct.gpio_pins = p.pin;
	gpio_init(p.port, &gpio_init_struct);

	/* clk/mo/mi MUXs */
	gpio_pin_mux_config(p.port, m[0].source, m[0].mux);
	gpio_pin_mux_config(p.port, m[1].source, m[1].mux);
	gpio_pin_mux_config(p.port, m[2].source, m[2].mux);
}

/**
 * @brief  Sets GPIO pin to low state
 */
static __inline void hal_stdpin_lo(void){
	gpio_bits_reset(CSPORT, CSPIN); //(csdata.port, csdata.pin);
}

/**
 * @brief  Sets GPIO pin to high state
 */
static __inline void hal_stdpin_hi(void){
	gpio_bits_set(CSPORT, CSPIN); //((csdata.port, csdata.pin);
}

// Inner - SPI Transfer Function
#define hal_spi_cmdxfer hal_spi_writexfer
static __inline uint8_t hal_spi_writexfer(uint8_t byte)
{
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_TDBE_FLAG) == RESET);
	spi_i2s_data_transmit(SPIDATA_SPI, byte);
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_RDBF_FLAG) == RESET);
	return (uint8_t)spi_i2s_data_receive(SPIDATA_SPI);
}
static __inline uint8_t hal_spi_readxfer(void)
{
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_TDBE_FLAG) == RESET);
	spi_i2s_data_transmit(SPIDATA_SPI, 0);
	while (spi_i2s_flag_get(SPIDATA_SPI, SPI_I2S_RDBF_FLAG) == RESET);
	return (uint8_t)spi_i2s_data_receive(SPIDATA_SPI);
}

// HAL - SPI Data Read Function
static __inline void hal_spi_data_read(uint8_t reg, uint8_t *pd)
{
	hal_spi_cmdxfer(reg); // | OPC_REG_R
	*pd = hal_spi_readxfer();
}

// HAL - SPI Data Write Function
static __inline void hal_spi_data_write(uint8_t reg, uint8_t val)
{
	hal_spi_cmdxfer(reg); // | OPC_REG_W
	hal_spi_writexfer(val);
}

static __inline void hal_spi_mem_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t i;
	hal_spi_cmdxfer(reg); // | OPC_REG_R
	for (i = 0; i < len; i++)
	buf[i] = hal_spi_readxfer();
}

static __inline void hal_spi_mem_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t i;
	hal_spi_cmdxfer(reg); // | OPC_REG_W
	for (i = 0; i < len; i++)
	hal_spi_writexfer(buf[i]);
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
