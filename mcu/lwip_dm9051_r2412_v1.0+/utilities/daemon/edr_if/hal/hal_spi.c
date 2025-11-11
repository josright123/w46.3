/**
 *******************************************************************************
 * @file    hal_spi.c
 * @brief   Hardware Abstraction Layer for DM9051 Ethernet Controller
 * 
 * @details This file provides functions for SPI initialization and communication with
 *          the DM9051 Ethernet controller.
 * 
 * @version 1.0.0
 * @author  Joseph CHANG
 * @copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * @date    2024-11-10
 *******************************************************************************
 */

#include "platform_info/control/drv_control/conf_core.h"
#include "platform_info/control/drv_control/dm9051_drv_debug.h"

/*******************************************************************************
 * Definitions and Macros
 ******************************************************************************/
/* SPI Commands */
#define DM9051_MRCMDX     (0x70)  /* Read_Mem2X */
#define DM9051_MRCMD      (0x72)  /* Read_Mem */
#define DM9051_MWCMD      (0x78)  /* Write_Mem */

// SPI Function Prototypes
#define dm9051_spi_command_write AT_spi_xfer
#define dm9051_spi_dummy_read() AT_spi_xfer(0)
uint8_t AT_spi_xfer(uint8_t byte);

#define AT_spi_data_read spi_data_read
#define AT_spi_data_write spi_data_write
#define AT_spi_mem2x_read spi_mem2x_read
#define AT_spi_mem_read spi_mem_read
#define AT_spi_mem_write spi_mem_write
void AT_spi_data_read(uint8_t reg, uint8_t *pd);
void AT_spi_data_write(uint8_t reg, uint8_t val);
void AT_spi_mem2x_read(uint8_t *pd);
void AT_spi_mem_read(uint8_t *buf, uint16_t len);
void AT_spi_mem_write(uint8_t *buf, uint16_t len);

void AT_hal_init(void); //(struct board_init_type *board_init_struct);
uint32_t AT_hal_tick_count(void);
void AT_hal_tick(void);

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
#define spi_set() spi_cset[0]
#define spi_number() spi_cset[0].spi

// SPI Configuration Structure
struct spi_config_t {
    spi_type *spi;                       // SPI instance
    crm_periph_clock_type clock;         // Peripheral clock type
    struct gpio_config_t sck;               // SCK pin configuration
    struct gpio_config_t miso;              // MISO pin configuration
    struct gpio_config_t mosi;              // MOSI pin configuration
};

/*******************************************************************************
 * Static Variables
 ******************************************************************************/
static const struct spi_config_t spi_cset[1] = {
    {
        SPI1,
        CRM_SPI1_PERIPH_CLOCK,
        {	
					GPIOA, 
					GPIO_PINS_5, 
					CRM_GPIOA_PERIPH_CLOCK, 
					{
						GPIO_PULL_NONE, 
						GPIO_MODE_MUX,
						GPIO_PINS_SOURCE5, GPIO_MUX_5
					}, 
				},
        {	
					GPIOA, 
					GPIO_PINS_6, 
					CRM_GPIOA_PERIPH_CLOCK, 
					{
						GPIO_PULL_NONE, 
						GPIO_MODE_MUX,
						GPIO_PINS_SOURCE6, GPIO_MUX_5
					}, 
				},
        {	
					GPIOA, 
					GPIO_PINS_7, 
					CRM_GPIOA_PERIPH_CLOCK, 
					{
						GPIO_PULL_NONE, 
						GPIO_MODE_MUX,
						GPIO_PINS_SOURCE7, GPIO_MUX_5
					}, 
				},
    }
};

// SPI Configuration Initialization
#define AT_spi_config_init dm9051if_spi_config
void AT_spi_config_init(const struct spi_config_t *config);

struct board_init_type dm_init_info; //temp here!

/*******************************************************************************
 * SPI Core Functions
 ******************************************************************************/
#if defined(_DLW_AT32F437xx)
// Get SPI Information Function
void dm9051_boards_get_info(struct board_init_type *board_init_struct)
{
	board_init_struct->spip = &spi_set();
	board_init_struct->sck = &spi_set().sck;
	board_init_struct->mi = &spi_set().miso;
	board_init_struct->mo = &spi_set().mosi;
	board_init_struct->csp = &cs_gpio;

	board_init_struct->interrrpt_mode = 0;
	board_init_struct->gpiop = intr_gpo();
	board_init_struct->intp = intr_set(); //=&intr_cset[0]
}

// SPI Initialization Function
void AT_hal_init(void) //(struct board_init_type *board_init_struct)
{
	dm9051_boards_get_info(&dm_init_info);

	#ifdef ETHERNET_INTERRUPT_MODE
	dm_init_info.interrrpt_mode = 1;
	#endif

    // Initialize SPI and GPIO configurations
    dm9051if_spi_config(dm_init_info.spip);
    dm9051if_stdpin_config(dm_init_info.sck);
    dm9051if_stdpin_config(dm_init_info.mi);
    dm9051if_stdpin_config(dm_init_info.mo);
    dm9051if_stdpin_config(dm_init_info.csp);

	AT_hal_intr_init(&dm_init_info);

	//if (dm_init_info.interrrpt_mode) {
	//}
}

// Interrupt Initialization Function
void AT_hal_intr_init(struct board_init_type *info)
{
	if (info->interrrpt_mode) {
		dm9051if_stdpin_config(info->gpiop);
		dm9051if_intr_config(info->intp);
	}
}

// SPI Configuration Function
void AT_spi_config_init(const struct spi_config_t *config)
{
    spi_init_type spi_init_struct;

    // Enable peripheral clock and initialize SPI
    crm_periph_clock_enable(config->clock, TRUE);
    spi_default_para_init(&spi_init_struct);
    
    spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
    spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
    spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_8;
    spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
    spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
    spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
    spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
    
    spi_init(config->spi, &spi_init_struct);
    spi_enable(config->spi, TRUE);
}

// SPI Transfer Function
uint8_t AT_spi_xfer(uint8_t byte)
{
    while (spi_i2s_flag_get(spi_number(), SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(spi_number(), byte);
    while (spi_i2s_flag_get(spi_number(), SPI_I2S_RDBF_FLAG) == RESET);
    return (uint8_t)spi_i2s_data_receive(spi_number());
}

// SPI Data Read Function
void AT_spi_data_read(uint8_t reg, uint8_t *pd)
{
    dm9051_spi_command_write(reg | OPC_REG_R);
    *pd = dm9051_spi_dummy_read();
}

// SPI Data Write Function
void AT_spi_data_write(uint8_t reg, uint8_t val) {
    dm9051_spi_command_write(reg | OPC_REG_W);
    dm9051_spi_command_write(val);
}

// Memory Read and Write Functions
void AT_spi_mem2x_read(uint8_t *pd) {
    dm9051_spi_command_write(DM9051_MRCMDX | OPC_REG_R);
    dm9051_spi_dummy_read();
    *pd = dm9051_spi_dummy_read();
}

void AT_spi_mem_read(uint8_t *buf, uint16_t len)
{
	uint16_t i;
    dm9051_spi_command_write(DM9051_MRCMD | OPC_REG_R);
    for (i = 0; i < len; i++)
        buf[i] = dm9051_spi_dummy_read();
}

void AT_spi_mem_write(uint8_t *buf, uint16_t len)
{
	uint16_t i;
    dm9051_spi_command_write(DM9051_MWCMD | OPC_REG_W);
    for (i = 0; i < len; i++)
        dm9051_spi_command_write(buf[i]);
}

/*******************************************************************************
 * Register Access Functions
 ******************************************************************************/
uint8_t cspi_read_reg(uint8_t reg)
{
    uint8_t val;
    dm9051if_stdpin_lo(&cs_gpio);
    spi_data_read(reg, &val);
    dm9051if_stdpin_hi(&cs_gpio);
    return val;
}

void cspi_write_reg(uint8_t reg, uint8_t val)
{
    dm9051if_stdpin_lo(&cs_gpio);
    spi_data_write(reg, val);
    dm9051if_stdpin_hi(&cs_gpio);
}

void cspi_read_regs_long(uint8_t reg, uint8_t *buf, uint16_t len) {
	uint16_t i;
    dm9051if_stdpin_lo(&cs_gpio); // Chip select low
    for (i = 0; i < len; i++, reg++) {
        spi_data_read(reg, &buf[i]);
    }
    dm9051if_stdpin_hi(&cs_gpio); // Chip select high
}

void cspi_read_regs_each(uint8_t reg, uint8_t *buf, uint16_t len) {
	uint16_t i;
    for (i = 0; i < len; i++, reg++) {
        buf[i] = cspi_read_reg(reg);
    }
}

// RX Buffer Read Function
uint8_t cspi_read_rxb(void) {
    uint8_t rxb;
    dm9051if_stdpin_lo(&cs_gpio); // Chip select low
    spi_mem2x_read(&rxb);
    dm9051if_stdpin_hi(&cs_gpio); // Chip select high
    return rxb;
}

/*******************************************************************************
 * Memory Access Functions
 ******************************************************************************/
void cspi_read_mem(uint8_t *buf, uint16_t len)
{
    dm9051if_stdpin_lo(&cs_gpio);
    spi_mem_read(buf, len);
    dm9051if_stdpin_hi(&cs_gpio);
}

void cspi_write_mem(uint8_t *buf, uint16_t len)
{
    dm9051if_stdpin_lo(&cs_gpio);
    spi_mem_write(buf, len);
    dm9051if_stdpin_hi(&cs_gpio);
}

/*******************************************************************************
 * Utility Functions
 ******************************************************************************/
// Tick Management
static uint32_t AT_HalTicks = 0;

uint32_t AT_hal_tick_count(void) {
	static uint32_t hal_tick_count = 0; 
	#if 1 //[Not only by SysTick counting][if take this, experiment to get 1000000 correction with your system.]
	hal_tick_count++;
	if (hal_tick_count >= 1000000) {
		hal_tick_count = 0;
		AT_HalTicks++;
	}
	#endif
    return AT_HalTicks;
}

// SysTick counting (NOT must essential)
void AT_hal_tick(void) {
    AT_HalTicks++;
}
#endif //_DLW_AT32F437xx

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
 
#if defined(_DLW_AT32F437xx)

#define AT_hal_irqline HAL_IRQLine

// ---------------------- data_impl -----------------------------------------------------------
// GPIO Data
const struct gpio_config_t
	cs_gpio = {
		GPIOA,
		GPIO_PINS_15,
		CRM_GPIOA_PERIPH_CLOCK,
		{
			GPIO_PULL_NONE,
			GPIO_MODE_OUTPUT,
		}
},
	intr_gpio
#ifdef ETHERNET_INTERRUPT_MODE
	= {
		GPIOC,
		GPIO_PINS_7,
		CRM_GPIOC_PERIPH_CLOCK,
		{
			GPIO_PULL_UP,
			GPIO_MODE_INPUT,
		}
}
#endif
;

const struct interrupt_pack_t intr_cset[1]
#ifdef ETHERNET_INTERRUPT_MODE
= {{
		{
			CRM_SCFG_PERIPH_CLOCK,
			CRM_GPIOC_PERIPH_CLOCK,
			SCFG_PORT_SOURCE_GPIOC,
			SCFG_PINS_SOURCE7,
			EXINT_LINE_7,
			NVIC_PRIORITY_GROUP_0,
			EXINT9_5_IRQn,
		},
		EXINT_TRIGGER_FALLING_EDGE,
	}}
#endif
	;

// ---------------------- hw_impl -------------------------------------------------------------
/**
 * @brief  Configures a standard GPIO pin
 * @param  gpio: Pointer to GPIO configuration structure
 * @note   This function handles:
 *         - Clock enabling
 *         - Pin mode configuration
 *         - Pull-up/down configuration
 *         - Drive strength setting
 */
void AT_hal_stdpin_config(const struct gpio_config_t *gpio)
{
	gpio_init_type gpio_init_struct;

	/* Enable peripheral clock for selected GPIO port */
	crm_periph_clock_enable(gpio->clock, TRUE);

	/* Initialize GPIO structure with default values */
	gpio_default_para_init(&gpio_init_struct);

	/* Configure GPIO parameters */
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = gpio->mods.mode;
	gpio_init_struct.gpio_pull = gpio->mods.pull;
	gpio_init_struct.gpio_pins = gpio->pin;

	/* Apply configuration */
	gpio_init(gpio->port, &gpio_init_struct);

	/* Configure multiplexing if in MUX mode */
#if defined(_DLW_AT32F437xx)
	if (gpio->mods.mode == GPIO_MODE_MUX)
		gpio_pin_mux_config(gpio->port, gpio->mods.source, gpio->mods.mux);
#endif /* _DLW_AT32F437xx */
}

// Static function prototypes
void AT_interrupt_config_init(const struct interrupt_pack_t *pack)
{
//#ifdef ETHERNET_INTERRUPT_MODE
//#endif
	//if (dm_init_info.interrrpt_mode) {
	//}
	const struct interrupt_config_t *cf = &(pack->cf);
	exint_polarity_config_type pol = pack->polarity; //EXINT_TRIGGER_FALLING_EDGE;
	exint_init_type exint_init_struct;

	/* config irq
	 */
	crm_periph_clock_enable(cf->scfgclock, TRUE);
	crm_periph_clock_enable(cf->clock, TRUE);

	scfg_exint_line_config(cf->port_source,
						   cf->pin_source);

	exint_default_para_init(&exint_init_struct);
	exint_init_struct.line_enable = TRUE;
	exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
	exint_init_struct.line_select = cf->line;
	exint_init_struct.line_polarity = pol;
	exint_init(&exint_init_struct);

	identify_irq_stat(ISTAT_IRQ_CFG);
	trace_irq_stat(ISTAT_IRQ_CFG);

	identify_irq_stat(ISTAT_LOW_TRIGGER);
	trace_irq_stat(ISTAT_LOW_TRIGGER);
}

/**
 * @brief  Sets GPIO pin to low state
 * @param  gpio: Pointer to GPIO configuration structure
 */
void AT_hal_stdpin_lo(const struct gpio_config_t *gpio)
{
	gpio_bits_reset(gpio->port, gpio->pin);
}

/**
 * @brief  Sets GPIO pin to high state
 * @param  gpio: Pointer to GPIO configuration structure
 */
void AT_hal_stdpin_hi(const struct gpio_config_t *gpio)
{
	gpio_bits_set(gpio->port, gpio->pin);
}

/**
 * @brief  Reads current state of GPIO pin
 * @param  gpio: Pointer to GPIO configuration structure
 * @return flag_status: SET if pin is high, RESET if pin is low
 */
flag_status AT_hal_stdpin_get(const struct gpio_config_t *gpio)
{
	return gpio_input_data_bit_read(gpio->port, gpio->pin);
}

void AT_hal_disable_mcu_irq(void)
{
	deidentify_irq_stat(ISTAT_IRQ_ENAB);
	nvic_irq_disable(dm_init_info.intp->cf.irqn); //(nvic_irqn())
}

void AT_hal_enable_mcu_irq(void)
{
	//if (dm_init_info.interrrpt_mode) {
	//}
	identify_irq_stat(ISTAT_IRQ_ENAB);
	trace_irq_stat(ISTAT_IRQ_ENAB);

	nvic_priority_group_config(dm_init_info.intp->cf.priority_group); //nvic_prio()
	nvic_irq_enable(dm_init_info.intp->cf.irqn, 1, 0);
}

uint32_t AT_hal_irqline(void)
{
	return dm_init_info.intp->cf.line; //irq_line();
}

#endif //_DLW_AT32F437xx
