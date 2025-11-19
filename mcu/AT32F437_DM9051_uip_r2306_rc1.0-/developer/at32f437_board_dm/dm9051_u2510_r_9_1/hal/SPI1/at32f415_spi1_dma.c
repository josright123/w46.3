/**
 *******************************************************************************
 * @file    at32f415_spi1_dma.c
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
 * @date    2025-09-08
 *******************************************************************************
 */
#include "hal/hal_at32f415.h"
#include "core/dm9051.h"

static char *spi_info[] = {
	"AT32F415 ETHERNET SPI1 DMA",
	"sck/mosi/miso/ pa5/pa7/pa6, cs/ pb12",
};

/* DMA parameters */
#define DMA_TX_CHANNEL			DMA1_CHANNEL1 //DMA1_CHANNEL3(NG. in-project)
#define DMA_RX_CHANNEL			DMA1_CHANNEL2
#define DMA_RX_FLAG				DMA1_FDT2_FLAG
#define SPI_MASTER_PROCESS_FLG	SPI_I2S_BF_FLAG

/* SPI and GPIO Configuration Macros for AT32F403A */
#define SPIDATA_CLK	    		CRM_SPI1_PERIPH_CLOCK
#define GPIOPORT_CLK			CRM_GPIOA_PERIPH_CLOCK
#define csGPIOPORT_CLK			CRM_GPIOB_PERIPH_CLOCK

#define SPIDATA_SPI	    		SPI1
#define PINS_CLKMOMI			{GPIOA, GPIO_PINS_5 | GPIO_PINS_7 | GPIO_PINS_6,}

#define CSPORT		    		GPIOB
#define CSPIN		    		GPIO_PINS_12

/* Static Configuration Data */
const static struct gpio_t p = 	PINS_CLKMOMI;

/**
 * @brief GPIO multiplexer configuration structure
 */
/* AT32F403A uses different MUX configuration compared to F437 */
//#define MUX_CK		    { GPIO_PINS_SOURCE5, GPIO_MUX_5,}
//#define MUX_MO		    { GPIO_PINS_SOURCE7, GPIO_MUX_5,}
//#define MUX_MI		    { GPIO_PINS_SOURCE6, GPIO_MUX_5,}
/* AT32F415 uses different MUX configuration compared to F403A */
//#define MUX_CK		  default
//#define MUX_MO		  default
//#define MUX_MI		  default
//struct multiplex_t {
//	gpio_pins_source_type source;
//	gpio_mux_sel_type mux;
//};
//const static struct multiplex_t m[] = {MUX_CK, MUX_MO, MUX_MI};

#define RXBUFF_OVERSIZE_LEN 1538
uint8_t dummy_buf[RXBUFF_OVERSIZE_LEN];

//linking...
//.\objects\template.axf: Error: L6218E: Undefined symbol dma_channel_enable (referred from at32f415_spi1_dma.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol dma_default_para_init (referred from at32f415_spi1_dma.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol dma_flag_get (referred from at32f415_spi1_dma.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol dma_flexible_config (referred from at32f415_spi1_dma.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol dma_init (referred from at32f415_spi1_dma.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol dma_reset (referred from at32f415_spi1_dma.o).

/**
 * @brief  Get SPI information string
 * @param  index: Information index (0 or 1)
 * @retval Pointer to information string
 */
char *hal_spi_info(int index)
{
	crm_clocks_freq_type crm_clocks_freq_struct = {0};
	crm_clocks_freq_get(&crm_clocks_freq_struct);		

	printf("[DRIVER %s mode] CLK:%d(sclk_freq) %d(apb2_freq) %d(SPI CLK) \r\n", //" %d", " %d(apb1_freq)"
		hal_active_interrupt_desc(),
		crm_clocks_freq_struct.sclk_freq,
		//crm_clocks_freq_struct.ahb_freq,
		crm_clocks_freq_struct.apb2_freq,
		//crm_clocks_freq_struct.apb1_freq,
		crm_clocks_freq_struct.apb2_freq / (2 << SPIDATA_SPI->ctrl1_bit.mdiv_l)
		);
//	printf("[DRIVER %s mode] SPI CLK use %s %dMhz, set to %dMhz\r\n", 
//		hal_active_interrupt_desc(), "apb2_freq", 
//		(crm_clocks_freq_struct.apb2_freq / 1000000),
//		(crm_clocks_freq_struct.apb2_freq / 1000000) / (2 << SPIDATA_SPI->ctrl1_bit.mdiv_l));
	return spi_info[index];
}

/**
 * @brief  Initialize SPI1 peripheral for DM9051 communication
 * @note   Configures SPI1 in master mode with 8-bit data frame
 * @param  none
 * @retval none
 */
void hal_spi_initialize(void)
{
	spi_init_type spi_init_struct;
	gpio_init_type gpio_init_struct;

#if 1
//	dma_config();
  dma_init_type dma_init_struct = {0};
    /* 開啟DMA時鐘 */
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  
    /* TX DMA 配置 */
  /* use dma1_channel1 as spi1 transmit channel */
  dma_reset(DMA_TX_CHANNEL);
  dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = 0; // 實際傳送時再設
    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_base_addr = 0; // 實際傳送時再設!
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&(SPI1->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA_TX_CHANNEL, &dma_init_struct);
  dma_flexible_config(DMA1, FLEX_CHANNEL1, DMA_FLEXIBLE_SPI1_TX);

    /* RX DMA 配置 */
  /* use dma1_channel2 as spi1 receive channel */
  dma_reset(DMA_RX_CHANNEL);
  dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = 0; // 實際傳送時再設
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_base_addr = 0; // 實際傳送時再設!
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&(SPI1->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA_RX_CHANNEL, &dma_init_struct);
  dma_flexible_config(DMA1, FLEX_CHANNEL2, DMA_FLEXIBLE_SPI1_RX);
#endif

	crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE); //Non-f437,iomux-config
	gpio_pin_remap_config(SWJTAG_GMUX_010, TRUE); //Non-f437,iomux-config

	/* Enable SPI1 peripheral clock */
	crm_periph_clock_enable(SPIDATA_CLK, TRUE);
	
	/* Initialize SPI configuration structure */
	spi_default_para_init(&spi_init_struct);
	
	/* Configure SPI1 parameters */
	spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
	spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
	spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_2; //SPI_MCLK_DIV_4; //SPI_MCLK_DIV_2; //SPI_MCLK_DIV_16; //SPI_MCLK_DIV_2;        /* SPI clock = PCLK/2 */
	spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
	spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
	spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;    /* CPOL = 0 */
	spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;        /* CPHA = 0 */

	/* Initialize and enable SPI1 */
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
	spi_init(SPIDATA_SPI, &spi_init_struct);

#if 1
  /* use dma transmit and receive */
  spi_i2s_dma_transmitter_enable(SPI1, TRUE); //DMA_TX_CHANNEL
  spi_i2s_dma_receiver_enable(SPI1, TRUE); //DMA_RX_CHANNEL
#endif
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

//#define hal_spi_start_rdreg_dma_xfer	hal_spi_start_dma_xfer
//#define hal_spi_start_wtreg_dma_xfer	hal_spi_start_dma_xfer
//#define hal_spi_start_cmd_dma_xfer	hal_spi_start_dma_xfer
//#define hal_spi_start_rdmem_dma_xfer	hal_spi_start_dma_xfer
//#define hal_spi_start_wtmem_dma_xfer	hal_spi_start_dma_xfer

static __inline void hal_spi_halt_dma_xfer(void) //(uint8_t reg, uint8_t *pd)
{
	dma_channel_enable(DMA_TX_CHANNEL, FALSE);
	dma_channel_enable(DMA_RX_CHANNEL, FALSE);
}

static __inline void hal_spi_start_dma_xfer(void) //(uint8_t reg, uint8_t *pd)
{
  //[dma_start_and_complete()]
//first,  dma_channel_enable(DMA_RX_CHANNEL, TRUE); /* rx, */
  dma_channel_enable(DMA_TX_CHANNEL, TRUE); /* tx, enable spi master dma to fill and get data= */
  dma_channel_enable(DMA_RX_CHANNEL, TRUE); /* rx, */

  /* wait master spi data receive end */
  /* wait master idle when communication end */
  while(dma_flag_get(DMA_RX_FLAG) == RESET);
  while(spi_i2s_flag_get(SPI1, SPI_MASTER_PROCESS_FLG) != RESET);
}

/* LOC setup
 *
 */
//#warning "hal_spi_cmd_LOC_INLINE_func() in case to be applied"
#define hal_spi_cmd_LOC_INLINE_func(tbuf, rbuf, len) \
	DMA_TX_CHANNEL->maddr = (uint32_t)tbuf; \
	DMA_TX_CHANNEL->dtcnt = len; \
	DMA_RX_CHANNEL->maddr = (uint32_t)rbuf; \
	DMA_RX_CHANNEL->dtcnt = len

/* Hardware Interface Functions - Public API */

/* CMD setup
 *
 */
#define hal_spi_cmd_setup_INLINE_func(tbuf) /* tested if apply this macro definition */ \
	hal_spi_halt_dma_xfer(); \
	hal_spi_cmd_LOC_INLINE_func(tbuf, dummy_buf, 1)

#define hal_spi_rdreg_setup(tbuf, rbuf) \
	hal_spi_halt_dma_xfer(); \
	hal_spi_cmd_LOC_INLINE_func(tbuf, rbuf, 2)

#define hal_spi_wtreg_setup(tbuf) \
	hal_spi_halt_dma_xfer(); \
	hal_spi_cmd_LOC_INLINE_func(tbuf, dummy_buf, 2)

#define hal_spi_mem_setup(tbuf, rbuf, len) \
	hal_spi_halt_dma_xfer(); \
	hal_spi_cmd_LOC_INLINE_func(tbuf, rbuf, len)

//linking...
//.\objects\template.axf: Error: L6218E: Undefined symbol hal_spi_info (referred from dm9051_beta.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol hal_spi_initialize (referred from dm9051_beta.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol hal_read_reg (referred from at32_emac.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol hal_write_reg (referred from dm9051_beta.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol hal_read_mem (referred from dm9051_beta.o).
//.\objects\template.axf: Error: L6218E: Undefined symbol hal_write_mem (referred from dm9051_beta.o).

/**
 * @brief  Read single register from DM9051
 * @param  reg: Register address
 * @retval Register value
 */
uint8_t hal_read_reg(uint8_t reg)
{
//uint8_t val;
//static int creg = 0; 
//if (creg < 3)
//printf("hal_read_reg, reg %02x\r\n", reg);
	uint8_t tbuf[2]= {reg | OPC_REG_R, };
	uint8_t fifo_rx[2];
	hal_stdpin_lo();

	hal_spi_rdreg_setup(tbuf, fifo_rx);
	hal_spi_start_dma_xfer();

	hal_stdpin_hi();
	return fifo_rx[1];
//if (creg < 3) {
//printf("hal_read_reg, val %02x\r\n", val);
//creg++;
//}
}

/**
 * @brief  Write single register to DM9051
 * @param  reg: Register address
 * @param  val: Value to write
 */
void hal_write_reg(uint8_t reg, uint8_t val)
{
	uint8_t tbuf[2] = {reg | OPC_REG_W, val};
	hal_stdpin_lo();

	hal_spi_wtreg_setup(tbuf);
	hal_spi_start_dma_xfer();

	hal_stdpin_hi();
}

/**
 * @brief  Read data from DM9051 memory
 * @param  buf: Buffer to store read data
 * @param  len: Number of bytes to read
 */
void hal_read_mem(uint8_t *buf, uint16_t len)
{
	uint8_t tbuf[2] = {DM9051_MRCMD | OPC_REG_R, };
	hal_stdpin_lo();
	
	hal_spi_cmd_setup_INLINE_func(tbuf);
	hal_spi_start_dma_xfer();

	hal_spi_mem_setup(&dummy_buf[0], buf, len);
	hal_spi_start_dma_xfer();

	hal_stdpin_hi();

//	if (len == 4)
//	  printf("rdmem len %u, [%02x][%02x][%02x][%02x]\r\n", len, buf[0], buf[1], buf[2], buf[3]);
}

void hal_stop_mem(void)
{
	uint8_t tbuf[2] = {DM9051_VIDL | OPC_REG_R, };
	hal_stdpin_lo();
	
	hal_spi_cmd_setup_INLINE_func(tbuf);
	hal_spi_start_dma_xfer();

	hal_stdpin_hi();
}

/**
 * @brief  Write data to DM9051 memory
 * @param  buf: Buffer containing data to write
 * @param  len: Number of bytes to write
 */
void hal_write_mem(uint8_t *buf, uint16_t len)
{
	uint8_t tbuf[2] = {DM9051_MWCMD | OPC_REG_W, };
	hal_stdpin_lo();

	hal_spi_cmd_setup_INLINE_func(tbuf);
	hal_spi_start_dma_xfer();

	hal_spi_mem_setup(buf, &dummy_buf[1], len);
	hal_spi_start_dma_xfer();

	hal_stdpin_hi();
}
