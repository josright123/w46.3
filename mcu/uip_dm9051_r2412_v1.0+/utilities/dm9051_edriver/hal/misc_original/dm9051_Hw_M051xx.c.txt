#include "dm9051_cboard_data_types.h"
#include "dm9051_cstate.h"
#include "../dm_eth.h"

#if defined(_DLW_M051xx)
/* ------------------------------- NU configuration ----------------------------------------- */
// --------------------- NU ----------------------------

void NU_DM9051_SPI_Configuration(void);
void NU_DM9051_Interript_Configuration(void);

void DM9051_Configuration_NU(void)
{
	NU_DM9051_SPI_Configuration();
	NU_DM9051_Interript_Configuration();
}

//---------------------------------------

void NU_DM9051_SPI_Configuration(void)
{
#if 1
	/* Enable SPI1 peripheral clock */
	CLK_EnableModuleClock(SPI1_MODULE);
	/* Select HCLK as the clock source of SPI1 */
	CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL1_SPI1_S_HCLK, MODULE_NoMsk);
	/* Reset IP */
	SYS_ResetModule(SPI1_RST);
	/* Setup SPI1 multi-function pins */
	SYS->P0_MFP = SYS_MFP_P04_SPISS1 | SYS_MFP_P05_MOSI_1 | SYS_MFP_P06_MISO_1 | SYS_MFP_P07_SPICLK1;
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init SPI                                                                                                */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Configure SPI1 as a master, SPI clock rate 200 KHz,
		 clock idle low, 32-bit transaction, drive output on falling clock edge and latch input on rising edge. */
	SPI_Open(SPI1, SPI_MASTER, SPI_MODE_0, 8, 25000000);
	/* Disable the automatic hardware slave selection function. Select the SPI1_SS pin and configure as low-active. */
	SPI_DisableAutoSS(SPI1);
	SPI_EnableFIFO(SPI1, 3, 3);
#endif
}

void NU_DM9051_Interript_Configuration(void)
{
#ifdef DM9051_DRIVER_INTERRUPT
	identify_irq_stat(ISTAT_IRQ_CFG);
	trace_irq_stat(ISTAT_IRQ_CFG);
	identify_irq_stat(ISTAT_LOW_TRIGGER);
	trace_irq_stat(ISTAT_LOW_TRIGGER);

	// add user's mcu irq configuration code here.
#endif
}
#endif

#if defined(_DLW_M051xx)
// --------------------- NU ----------------------------
//[NU cspi]
void NU_spi_cs_lo(void)
{
	SPI_SET_SS_LOW(SPI1);
}
void NU_spi_cs_hi(void)
{
	SPI_SET_SS_HIGH(SPI1);
}
void NU_spi_exc_data_head(uint8_t byte)
{
	SPI_WRITE_TX0(SPI1, (uint32_t)byte);
	while (SPI_IS_BUSY(SPI1))
		;
	SPI_READ_RX0(SPI1);
}
uint8_t NU_spi_exc_data_read(void)
{
	SPI_WRITE_TX0(SPI1, 0x0); // Dummy for read register value.
	while (SPI_GET_RX_FIFO_EMPTY_FLAG(SPI1))
		;
	return (SPI_READ_RX0(SPI1) & 0xFF);
}
void NU_spi_exc_data_read_end(void)
{
	while (SPI_IS_BUSY(SPI1))
		;
	// Clear SPI TX FIFO
	SPI_ClearTxFIFO(SPI1);
}

void NU_spi_exc_data_write(uint8_t byte)
{
	while (SPI_GET_TX_FIFO_FULL_FLAG(SPI1))
		;
	SPI_WRITE_TX0(SPI1, (uint32_t)byte);
}
void NU_spi_exc_data_write_end(void)
{
	while (SPI_IS_BUSY(SPI1))
		;
	// Clear SPI RX FIFO
	SPI_ClearRxFIFO(SPI1);
}
#endif

// ---------------------- data_impl -------------------------------------------------------------
#define DM9051_MRCMDX (0x70) // Read_Mem2X
#define DM9051_MRCMD (0x72)	 // Read_Mem
#define DM9051_MWCMD (0x78)	 // Write_Mem

#define OPC_REG_W 0x80 // Register Write
#define OPC_REG_R 0x00 // Register Read

#if defined(_DLW_M051xx)
//[NU cspi_io]
#define dm9051_spi_cmd_write(wb) NU_spi_exc_data_head(wb)
#define dm9051_spi_data_read() NU_spi_exc_data_read()
#define dm9051_spi_read_end() NU_spi_exc_data_read_end()
#define dm9051_spi_data_write(wb) NU_spi_exc_data_write(wb)
#define dm9051_spi_write_end() NU_spi_exc_data_write_end()

uint8_t NU_spi_data_read(uint8_t reg)
{
	uint8_t val;
	dm9051_spi_cmd_write(reg | OPC_REG_R);
	val = dm9051_spi_data_read();
	dm9051_spi_read_end();
	return val;
}
void NU_spi_data_write(uint8_t reg, uint8_t val)
{
	dm9051_spi_cmd_write(reg | OPC_REG_W);
	dm9051_spi_data_write(val);
	dm9051_spi_write_end();
}

uint8_t NU_spi_mem2x_read(void)
{
	uint8_t rxb;
	rxb = cspi_read_reg(DM9051_MRCMDX);
	rxb = cspi_read_reg(DM9051_MRCMDX);
	return rxb;
}

void NU_spi_mem_read(uint8_t *buf, uint16_t len)
{
	int i;
	dm9051_spi_cmd_write(DM9051_MRCMD | OPC_REG_R);
	for (i = 0; i < len; i++)
		buf[i] = dm9051_spi_data_read();
	dm9051_spi_read_end();
}

void NU_spi_mem_write(uint8_t *buf, uint16_t len)
{
	int i;
	dm9051_spi_cmd_write(DM9051_MWCMD | OPC_REG_W);
	for (i = 0; i < len; i++)
		dm9051_spi_data_write(buf[i]);
	dm9051_spi_write_end();
}
#endif

// --------------------- AT ----------------------------
#if defined(_DLW_AT32F437xx)
#endif

#if defined(_DLW_AT32F437xx)
// --------------------- AT ----------------------------
//[AT cspi]
#endif

#if defined(_DLW_M051xx)
// --------------------- NU ----------------------------
void NU_cint_disable_mcu_irq(void)
{
#ifdef DM9051_DRIVER_INTERRUPT
	deidentify_irq_stat(ISTAT_IRQ_ENAB);

	// add user's mcu irq enable control code here.

#endif
}

void NU_cint_enable_mcu_irq(void)
{
#ifdef DM9051_DRIVER_INTERRUPT
	identify_irq_stat(ISTAT_IRQ_ENAB);
	trace_irq_stat(ISTAT_IRQ_ENAB);

	// add user's mcu irq enable control code here.

#endif
}

/* IRQ handler support */
int NU_cint_exint9_5_handler(void)
{

	identify_irq_stat(ISTAT_IRQ_NOW);
	trace_irq_stat(ISTAT_IRQ_NOW);

	// add user's mcu irq-handler source code here.

	// if(exint_flag_get(EXINT_LINE_7) != RESET) {

	identify_irq_stat(ISTAT_IRQ_NOW2);
	trace_irq_stat(ISTAT_IRQ_NOW2);

	// DM_ETH_ToSet_InterruptEvent();

	//	exint_flag_clear(EXINT_LINE_7);
	//}

	deidentify_irq_stat(ISTAT_IRQ_NOW | ISTAT_IRQ_NOW2);
	return 1;
}
#endif

// --------------------- AT ----------------------------
#if defined(_DLW_AT32F437xx)
#endif

#if 1
#include "clock.h"
#define sys_now clock_time // or xTaskGetTickCount()

void dm_delay_us(uint32_t nus)
{
	uint32_t startTick = sys_now();
	while ((sys_now() - startTick) < ((nus + 999) / 1000))
	{
		// Wait until the specified delay has passed
	}
}

void dm_delay_ms(uint16_t nms)
{
	uint32_t startTick = sys_now();
	while ((sys_now() - startTick) < nms)
	{
		// Wait until the specified delay has passed
	}
}
#endif
