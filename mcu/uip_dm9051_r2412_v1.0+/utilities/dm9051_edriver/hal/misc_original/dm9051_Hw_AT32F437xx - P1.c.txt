/**
 **************************************************************************
 * The use of structures for SPI and interrupt configurations allows
 *	for easy modification if needed.
 * There are separate functions for SPI initialization (spi_add) and
 *	interrupt initialization (intr_add).
 * The code includes status tracking functions (identify_irq_stat, trace_irq_stat)
 *	for debugging purposes.
 **************************************************************************
 */
#include "dm9051.h"
#include "../dm_eth_opts.h" //using 'freeRTOS'

#if defined(_DLW_AT32F437xx)

/* ------------------------------- AT configuration ----------------------------------------- */
// --------------------- AT ----------------------------

// SPI Configuration Structure
struct spi_config_t
{
	spi_type *spi;
	crm_periph_clock_type clock;
	struct gpio_mux_t
	{
		gpio_type *port;
		uint16_t pin;
		gpio_pins_source_type source;
		gpio_mux_sel_type mux;
		crm_periph_clock_type clock;
		gpio_mode_type mode;
	} sck, miso, mosi, cs;
};

// Interrupt Configuration Structure
struct interrupt_config_t
{
	struct
	{
		gpio_type *port;
		uint16_t pin;
		crm_periph_clock_type clock;
		gpio_mode_type mode;
	} gpio;
	struct
	{
		crm_periph_clock_type clock;
		scfg_port_source_type port_source;
		scfg_pins_source_type pin_source;
	} scfg;
	struct
	{
		crm_periph_clock_type clock;
		uint32_t line;
	} exti;
	struct
	{
		IRQn_Type irqn;
		nvic_priority_group_type priority_group;
	} nvic;
};

static const struct spi_config_t spi_cset[1] = {
	{
		SPI1,
		CRM_SPI1_PERIPH_CLOCK,
		{GPIOA, GPIO_PINS_5, GPIO_PINS_SOURCE5, GPIO_MUX_5, CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_MUX},
		{GPIOA, GPIO_PINS_6, GPIO_PINS_SOURCE6, GPIO_MUX_5, CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_MUX},
		{GPIOA, GPIO_PINS_7, GPIO_PINS_SOURCE7, GPIO_MUX_5, CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_MUX},
		{GPIOA, GPIO_PINS_15, (gpio_pins_source_type)0, (gpio_mux_sel_type)0, CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_OUTPUT},
	}};

static const struct interrupt_config_t spi_cintr[1] = {
	{
		{GPIOC, GPIO_PINS_7, CRM_GPIOC_PERIPH_CLOCK, GPIO_MODE_INPUT},
		{CRM_SCFG_PERIPH_CLOCK, SCFG_PORT_SOURCE_GPIOC, SCFG_PINS_SOURCE7},
		{CRM_GPIOC_PERIPH_CLOCK, EXINT_LINE_7},
		{EXINT9_5_IRQn, NVIC_PRIORITY_GROUP_0},
	}};

static void spi_config_init(const struct spi_config_t *config);
static void interrupt_config_init(const struct interrupt_config_t *config);

void dm9051_boards_initialize_AT(void)
{
	spi_config_init(spi_cset);
	// spi_add();
#ifdef DM9051_DRIVER_INTERRUPT
	interrupt_config_init(spi_cintr);
	// intr_add();
#endif
}

static void configure_cspi(const struct spi_config_t *ss)
{
	spi_init_type spi_init_struct;

	crm_periph_clock_enable(ss->clock, TRUE);

	spi_default_para_init(&spi_init_struct);
	spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
	spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
	spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_8;
	// spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_LSB;
	spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
	spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
	spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
	// spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
	spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
	spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
	spi_init(ss->spi, &spi_init_struct);
	spi_enable(ss->spi, TRUE);
}

static void configure_cpin(const struct gpio_mux_t *ps, gpio_pull_type gppull)
{
	gpio_init_type gpio_init_struct;

	/* enable the gpio clock
	 */
	crm_periph_clock_enable(ps->clock, TRUE);

	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = ps->mode;
	gpio_init_struct.gpio_pull = gppull; // GPIO_PULL_DOWN; GPIO_PULL_UP; //GPIO_PULL_NONE;
	gpio_init_struct.gpio_pins = ps->pin;
	gpio_init(ps->port, &gpio_init_struct);

#if defined(_DLW_AT32F437xx)
	if (ps->mode == GPIO_MODE_MUX)
		gpio_pin_mux_config(ps->port, ps->source, ps->mux);
#endif
}

// Function prototypes
// #if 1
static void spi_config_init(const struct spi_config_t *config)
{
	configure_cspi(config);
	configure_cpin(&config->sck, GPIO_PULL_NONE);
	configure_cpin(&config->miso, GPIO_PULL_NONE);
	configure_cpin(&config->mosi, GPIO_PULL_NONE);
	configure_cpin(&config->cs, GPIO_PULL_NONE);
}

static void configure_cirq(const struct interrupt_config_t *cf, gpio_pull_type gppull,
						   exint_polarity_config_type polarity)
{
	gpio_init_type gpio_init_struct;
	exint_init_type exint_init_struct;
#if 1
	// [Not all devices essential]
	// configure_pin(&cf->gpio, gppull);

	/* enable the gpio clock
	 */
	crm_periph_clock_enable(cf->gpio.clock, TRUE);

	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = cf->gpio.mode;
	gpio_init_struct.gpio_pull = gppull; // GPIO_PULL_DOWN; GPIO_PULL_UP; //GPIO_PULL_NONE;
	gpio_init_struct.gpio_pins = cf->gpio.pin;
	gpio_init(cf->gpio.port, &gpio_init_struct);

// [Not all devices essential]
// log_intr_qpio_pin_config();
#endif

	crm_periph_clock_enable(cf->scfg.clock, TRUE);
	crm_periph_clock_enable(cf->exti.clock, TRUE);

	scfg_exint_line_config(cf->scfg.port_source,
						   cf->scfg.pin_source);

	exint_default_para_init(&exint_init_struct);
	exint_init_struct.line_enable = TRUE;
	exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
	exint_init_struct.line_select = cf->exti.line;
	exint_init_struct.line_polarity = polarity;
	exint_init(&exint_init_struct);
}

static void interrupt_config_init(const struct interrupt_config_t *config)
{
	configure_cirq(config, GPIO_PULL_UP, EXINT_TRIGGER_FALLING_EDGE);
	//	#if 1
	// [Not all devices essential]
	// log_intr_qpio_pin_config();
	//	configure_pin(&spi_intr[0].gpio, GPIO_PULL_UP);
	//	#endif
	//	configure_irq(&spi_intr[0], EXINT_TRIGGER_FALLING_EDGE);

	identify_irq_stat(ISTAT_IRQ_CFG);
	trace_irq_stat(ISTAT_IRQ_CFG);
	identify_irq_stat(ISTAT_LOW_TRIGGER);
	trace_irq_stat(ISTAT_LOW_TRIGGER);
}
// #endif

// ... AT32F437xx specific interrupt enable/disable functions ...

// #if defined(_DLW_AT32F437xx)
//  --------------------- AT ----------------------------
void cint_disable_mcu_irq_AT(void)
{
#ifdef DM9051_DRIVER_INTERRUPT
	deidentify_irq_stat(ISTAT_IRQ_ENAB);
	nvic_irq_disable(spi_cintr[0].nvic.irqn);
#endif
}

void cint_enable_mcu_irq_AT(void)
{
#ifdef DM9051_DRIVER_INTERRUPT
	identify_irq_stat(ISTAT_IRQ_ENAB);
	trace_irq_stat(ISTAT_IRQ_ENAB);

	nvic_priority_group_config(spi_cintr[0].nvic.priority_group);
	nvic_irq_enable(spi_cintr[0].nvic.irqn, 1, 0);
#endif
}

/* IRQ handler support */
int cint_exint9_5_handler_AT(void)
{
	identify_irq_stat(ISTAT_IRQ_NOW);
	trace_irq_stat(ISTAT_IRQ_NOW);

	// add user's mcu irq-handler source code here.

	if (exint_flag_get(spi_cintr[0].exti.line) != RESET)
	{

		identify_irq_stat(ISTAT_IRQ_NOW2);
		trace_irq_stat(ISTAT_IRQ_NOW2);
		exint_flag_clear(spi_cintr[0].exti.line);

		// DM_ETH_SetInterruptEvent();
		return 1;
	}

	deidentify_irq_stat(ISTAT_IRQ_NOW | ISTAT_IRQ_NOW2);
	return 0;
}
// #endif

#define spi_number() spi_cset[0].spi
#define cs_gpio_port() spi_cset[0].cs.port // #define pin_cs() spi_cset[0].cs
#define cs_gpio_pin() spi_cset[0].cs.pin

// --------------------- AT ----------------------------
// #if defined(_DLW_AT32F437xx)
void AT_spi_cs_lo(void)
{
	gpio_bits_reset(cs_gpio_port(), cs_gpio_pin());
}
void AT_spi_cs_hi(void)
{
	gpio_bits_set(cs_gpio_port(), cs_gpio_pin());
}

uint8_t AT_spi_exc_data(uint8_t byte)
{
	while (spi_i2s_flag_get(spi_number(), SPI_I2S_TDBE_FLAG) == RESET)
		;
	spi_i2s_data_transmit(spi_number(), byte);
	while (spi_i2s_flag_get(spi_number(), SPI_I2S_RDBF_FLAG) == RESET)
		;
	return (uint8_t)spi_i2s_data_receive(spi_number());
}
// #endif

// ---------------------- data_impl -------------------------------------------------------------
#define DM9051_MRCMDX (0x70) // Read_Mem2X
#define DM9051_MRCMD (0x72)	 // Read_Mem
#define DM9051_MWCMD (0x78)	 // Write_Mem

#define OPC_REG_W 0x80 // Register Write
#define OPC_REG_R 0x00 // Register Read

//[AT cspi_io]
// #if defined(_DLW_AT32F437xx)
#define dm9051_spi_command_write(rd) AT_spi_exc_data(rd)
#define dm9051_spi_dummy_read() AT_spi_exc_data(0)

uint8_t AT_spi_data_read(uint8_t reg)
{
	dm9051_spi_command_write(reg | OPC_REG_R);
	return dm9051_spi_dummy_read();
}
void AT_spi_data_write(uint8_t reg, uint8_t val)
{
	dm9051_spi_command_write(reg | OPC_REG_W);
	dm9051_spi_command_write(val);
}

uint8_t AT_spi_mem2x_read(void)
{
	dm9051_spi_command_write(DM9051_MRCMDX | OPC_REG_R);
	dm9051_spi_dummy_read();
	return dm9051_spi_dummy_read();
}
void AT_spi_mem_read(uint8_t *buf, uint16_t len)
{
	int i;
	dm9051_spi_command_write(DM9051_MRCMD | OPC_REG_R);
	for (i = 0; i < len; i++)
		buf[i] = dm9051_spi_dummy_read();
}
void AT_spi_mem_write(uint8_t *buf, uint16_t len)
{
	int i;
	dm9051_spi_command_write(DM9051_MWCMD | OPC_REG_W);
	for (i = 0; i < len; i++)
		dm9051_spi_command_write(buf[i]);
}
// #endif
// #if defined(_DLW_AT32F437xx)
#define spi_datas_read(r, b, l) AT_spi_datas_read(r, b, l)
#define spi_datas_write(r, b, l) AT_spi_datas_write(r, b, l)

void AT_spi_datas_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
	int i;
	for (i = 0; i < len; i++, reg++)
	{
		dm9051_spi_command_write(reg | OPC_REG_R);
		buf[i] = dm9051_spi_dummy_read();
	}
}
void AT_spi_datas_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
	int i;
	for (i = 0; i < len; i++, reg++)
	{
		dm9051_spi_command_write(reg | OPC_REG_W);
		dm9051_spi_command_write(buf[i]);
	}
}
// #endif
#endif //_DLW_AT32F437xx

/* dm9051_Hw_common implementation
 * source code.
 */
uint8_t cspi_read_reg(uint8_t reg) // static (todo)
{
	uint8_t val;
	dm9051if_cs_lo();
	val = spi_data_read(reg);
	dm9051if_cs_hi();
	return val;
}

void cspi_write_reg(uint8_t reg, uint8_t val)
{
	dm9051if_cs_lo();
	spi_data_write(reg, val);
	dm9051if_cs_hi();
}

void cspi_read_regs(uint8_t reg, uint8_t *buf, uint16_t len, csmode_t csmode)
{
	int i;

	if (csmode == CS_LONG)
	{
		dm9051if_cs_lo();
		spi_datas_read(reg, buf, len);
		dm9051if_cs_hi();
	}
	else
	{ // CS_EACH
		for (i = 0; i < len; i++, reg++)
			buf[i] = cspi_read_reg(reg);
	}
}

void cspi_write_regs(uint8_t reg, uint8_t *buf, uint16_t len, csmode_t csmode)
{
	int i;

	if (csmode == CS_LONG)
	{
		dm9051if_cs_lo();
		spi_datas_write(reg, buf, len);
		dm9051if_cs_hi();
	}
	else
	{ // CS_EACH
		for (i = 0; i < len; i++, reg++)
			cspi_write_reg(reg + i, buf[i]);
	}
}

uint8_t cspi_read_mem2x(void)
{
	uint8_t rxb;
	dm9051if_cs_lo();
	rxb = spi_mem2x_read();
	dm9051if_cs_hi();
	return rxb;
}
void cspi_read_mem(uint8_t *buf, uint16_t len)
{
	dm9051if_cs_lo();
	spi_mem_read(buf, len);
	dm9051if_cs_hi();
}
void cspi_write_mem(uint8_t *buf, uint16_t len)
{
	dm9051if_cs_lo();
	spi_mem_write(buf, len);
	dm9051if_cs_hi();
}

/* dm9051_Hw_common delay funcrions
 * Select ether [0] or [1], or otherwise coding user's sys_now().
 */
#if freeRTOS
//(1)in case freertos
// #include "lwip/sys.h"
#define sys_now xTaskGetTickCount
#else
//(2)[in case of uip]
#include "clock.h"
#define sys_now clock_time // or check uip
//(3)[in case of lwip]
#define sys_now sys_now	   // to check lwip
#endif

void dm_delay_us(uint32_t nus)
{
	uint32_t start = sys_now();
	while ((sys_now() - start) < ((nus + 999) / 1000))
		;
}

void dm_delay_ms(uint16_t nms)
{
	uint32_t start = sys_now();
	while ((sys_now() - start) < nms)
		;
}

#if 0


struct spi_pin_t
{
	gpio_type *gpport; //= PORT;
	uint16_t pin;	   //= PIN;
	gpio_pins_source_type pinsrc;
	gpio_mux_sel_type muxsel;
};

struct spi_pin_set_t
{
	crm_periph_clock_type clock;
	gpio_mode_type gpio_mode;
	struct spi_pin_t gpio_pin;
};

struct spi_set_t
{
	// char *spi_desc;
	spi_type *spi;				 //= SPIPORT;
	crm_periph_clock_type clock; //= SPI_CRM_CLK;
	struct spi_pin_set_t pin_sck;
	struct spi_pin_set_t pin_miso;
	struct spi_pin_set_t pin_mosi;
	struct spi_pin_set_t pin_cs; /* //CS (e.g. PA15) */
};

struct irq_set_t
{
	crm_periph_clock_type clock;		 // CRM_SCFG_PERIPH_CLOCK,
	scfg_port_source_type scfg_port_src; // SCFG_PORT_SOURCE_X
	scfg_pins_source_type scfg_pin_src;	 // SCFG_PINS_SOURCEX
};

struct irqline_set_t
{
	crm_periph_clock_type clock; // CRM_GPIOC_PERIPH_CLOCK,
	uint32_t extline;			 //= LINE
};

struct interrupt_mode_t
{
	IRQn_Type irqn;					   //= EXINTn_m
	nvic_priority_group_type priority; //_group;
};

struct interrupt_set_t
{
	struct spi_pin_set_t gpio; /* //INTR (e.g. PC7) */
	struct irq_set_t irq;
	struct irqline_set_t irqline;
	struct interrupt_mode_t interrupt;
};

// ... SPI and GPIO structure definitions ...

static const struct spi_set_t spi_set[1] = {
	{SPI1,
	 CRM_SPI1_PERIPH_CLOCK,
	 {CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_MUX, {GPIOA, GPIO_PINS_5, GPIO_PINS_SOURCE5, GPIO_MUX_5}},
	 {CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_MUX, {GPIOA, GPIO_PINS_6, GPIO_PINS_SOURCE6, GPIO_MUX_5}},
	 {CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_MUX, {GPIOA, GPIO_PINS_7, GPIO_PINS_SOURCE7, GPIO_MUX_5}},
	 {CRM_GPIOA_PERIPH_CLOCK, GPIO_MODE_OUTPUT, {GPIOA, GPIO_PINS_15}}}};

// ... Interrupt structure definitions ...

static const struct interrupt_set_t spi_intr[1] = {
	{
		//        .gpio = {
		//            .clock = CRM_GPIOC_PERIPH_CLOCK,
		//            .gpio_mode = GPIO_MODE_INPUT,
		//            .gpio_pin = {GPIOC, GPIO_PINS_7}
		//        },
		//        .irq = {
		//            .clock = CRM_SCFG_PERIPH_CLOCK,
		//            .scfg_port_src = SCFG_PORT_SOURCE_GPIOC,
		//            .scfg_pin_src = SCFG_PINS_SOURCE7
		//        },
		//        .irqline = {
		//            .clock = CRM_GPIOC_PERIPH_CLOCK,
		//            .extline = EXINT_LINE_7
		//        },
		//        .interrupt = {
		//            .irqn = EXINT9_5_IRQn,
		//            .priority = NVIC_PRIORITY_GROUP_0
		//        }
		{CRM_GPIOC_PERIPH_CLOCK, GPIO_MODE_INPUT, {GPIOC, GPIO_PINS_7}},
		{CRM_SCFG_PERIPH_CLOCK, SCFG_PORT_SOURCE_GPIOC, SCFG_PINS_SOURCE7},
		{CRM_GPIOC_PERIPH_CLOCK, EXINT_LINE_7},
		{EXINT9_5_IRQn, NVIC_PRIORITY_GROUP_0},

	}};

static void configure_spi(const struct spi_set_t *ss)
{
	spi_init_type spi_init_struct;

	crm_periph_clock_enable(ss->clock, TRUE);

	spi_default_para_init(&spi_init_struct);
	spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
	spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
	spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_8;
	// spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_LSB;
	spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
	spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
	spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
	// spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
	spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
	spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
	spi_init(ss->spi, &spi_init_struct);
	spi_enable(ss->spi, TRUE);
}

static void configure_pin(const struct spi_pin_set_t *ps, gpio_pull_type gppull)
{
	gpio_init_type gpio_init_struct;

	/* enable the gpio clock
	 */
	crm_periph_clock_enable(ps->clock, TRUE);

	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_mode = ps->gpio_mode;
	gpio_init_struct.gpio_pull = gppull; // GPIO_PULL_DOWN; GPIO_PULL_UP; //GPIO_PULL_NONE;
	gpio_init_struct.gpio_pins = ps->gpio_pin.pin;
	gpio_init(ps->gpio_pin.gpport, &gpio_init_struct);

#if defined(_DLW_AT32F437xx)
	if (ps->gpio_mode == GPIO_MODE_MUX)
		gpio_pin_mux_config(ps->gpio_pin.gpport, ps->gpio_pin.pinsrc, ps->gpio_pin.muxsel);
#endif
}

// ... GPIO and SPI configuration functions ...

void spi_add(void)
{
	configure_spi(&spi_set[0]);
	configure_pin(&spi_set[0].pin_sck, GPIO_PULL_NONE);
	configure_pin(&spi_set[0].pin_miso, GPIO_PULL_NONE);
	configure_pin(&spi_set[0].pin_mosi, GPIO_PULL_NONE);
	configure_pin(&spi_set[0].pin_cs, GPIO_PULL_NONE);
}

static void configure_irq(const struct interrupt_set_t *spiintr,
						  exint_polarity_config_type polarity)
{ // const struct extscfg_st *pexint_set,
	exint_init_type exint_init_struct;

	crm_periph_clock_enable(spiintr->irq.clock, TRUE);
	crm_periph_clock_enable(spiintr->irqline.clock, TRUE);

	// #ifndef AT32F437xx
	//   gpio_exint_line_config(scfg_port(), scfg_pin()); // SCFG_PORT_SOURCE_GPIOA, SCFG_PINS_SOURCE0
	// #else
	// #endif
	scfg_exint_line_config(spiintr->irq.scfg_port_src,
						   spiintr->irq.scfg_pin_src);

	exint_default_para_init(&exint_init_struct);
	exint_init_struct.line_enable = TRUE;
	exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
	exint_init_struct.line_select = spiintr->irqline.extline;
	exint_init_struct.line_polarity = polarity; // EXINT_TRIGGER_RISING_EDGE/ EXINT_TRIGGER_FALLING_EDGE
	exint_init(&exint_init_struct);
}

// ... Interrupt configuration structures and functions ...

#ifdef DM9051_DRIVER_INTERRUPT
void intr_add(void)
{
#if 1
	// [Not all devices essential]
	// log_intr_qpio_pin_config();
	configure_pin(&spi_intr[0].gpio, GPIO_PULL_UP);
#endif
	configure_irq(&spi_intr[0], EXINT_TRIGGER_FALLING_EDGE); //[ops] //&devconf_at437_intr_c7.extend1,

	identify_irq_stat(ISTAT_IRQ_CFG);
	trace_irq_stat(ISTAT_IRQ_CFG);
	identify_irq_stat(ISTAT_LOW_TRIGGER);
	trace_irq_stat(ISTAT_LOW_TRIGGER);
}
#endif // DM9051_DRIVER_INTERRUPT
#endif
