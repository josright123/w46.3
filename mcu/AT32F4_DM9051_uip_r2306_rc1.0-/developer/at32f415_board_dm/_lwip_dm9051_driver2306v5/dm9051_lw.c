/*
 * Copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This DM9051 Driver for LWIP tcp/ip stack
 * First veryification with : AT32F415
 *
 * Author: Joseph CHANG <joseph_chang@davicom.com.tw>
 * Date: 20230411
 * Date: 20230428 (V3)
 */
#include "stdint.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "developer_conf.h"
#include "dm9051_lw.h"
/* SPI master control is essential,
 * Do define spi specific definition, depend on your CPU's board support package library.
 * Here we had the definition header file with : "at32f415_spi.h"
 * for AT32F415 cpu.
 */
#include "at32f415_spi.h" //#ifdef SPI_MODULE_ENABLED (#include xxx_spi.h) #endif
static void dm9051_core_reset(void);
static void dm9051_set_par(uint8_t *macadd);
static void dm9051_set_mar(void);
static void dm9051_set_recv(void);

static u8 lw_flag;
static u8 link_reset = 1;
static u16 unlink_count;

#define UNLINK_COUNT_RESET	60000

//int board_printf(const char *format, args...) {
//	return 0;
//}
#define	board_printf(format, args...)

//static void lwip_set_mac_address(unsigned char* macadd);
//static void lwip_get_mac_address(unsigned char *adr);

static void dm_delay_us(uint32_t nus)
{
	void delay_us(uint32_t nus);
	board_printf("test %d ,because rxb %02x (is %d times)\r\n", rstccc, rxbyteee, timesss);
	delay_us(nus);
}
static void dm_delay_ms(uint16_t nms)
{
	void delay_ms(uint16_t nms);
	delay_ms(nms);
	
}

#if 1
static uint8_t MACaddr[6];
static void lwip_set_mac_address(uint8_t* macadd)
{
  MACaddr[0] = macadd[0];
  MACaddr[1] = macadd[1];
  MACaddr[2] = macadd[2];
  MACaddr[3] = macadd[3];
  MACaddr[4] = macadd[4];
  MACaddr[5] = macadd[5];
}

static void lwip_get_mac_address(uint8_t *adr)
{
  memcpy(adr, MACaddr, 6);
}
#endif

uint8_t DM9051_Read_Reg(uint8_t reg) //static (todo)
{
	union {
	uint8_t rxpad;
	uint8_t val;
	} rd;
	SPI_DM9051_CS_LOW();
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, reg); //spi2 tx	
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    rd.rxpad = (uint8_t) spi_i2s_data_receive(SPI2); //spi2 rx (rx_pad)

	//printf("[%02x]_%02x) spi2\r\n", reg, rxpad);
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, 0); //spi2 tx (tx_pad with '0x00')
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    rd.val = (uint8_t) spi_i2s_data_receive(SPI2); //spi2 rx
	//printf("[..][%02x]\r\n", val); //from dm9051
	SPI_DM9051_CS_HIGH();
	return rd.val;
}
//static 
void DM9051_Write_Reg(uint8_t reg, uint8_t val)
{
	/*u8 rxpad;*/
	SPI_DM9051_CS_LOW();
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, reg | 0x80); //spi2 tx-cmd
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    /*rxpad = (u8)*/ spi_i2s_data_receive(SPI2); //spi2 rx (rx_pad)

	//printf("[%02x]_%02x) spi2\r\n", reg, rxpad);
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, val); //spi2 tx-val
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    /*rxpad = (u8)*/ spi_i2s_data_receive(SPI2); //spi2 rx (rx_pad)
	//printf("[%02x]_%02x) spi2\r\n", reg, rxpad);
	SPI_DM9051_CS_HIGH();
}
static uint8_t DM9051_Read_Mem2X(void)
{
	uint16_t i;
	union {
	uint8_t un;
	uint8_t rxb;
	} rd;
	SPI_DM9051_CS_LOW();
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, DM9051_MRCMDX); //spi2 tx-cmd
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    /*rd.v0 = (u8)*/ spi_i2s_data_receive(SPI2); //spi2 rx (rx_pad)

	for(i=0; i<2; i++) {
		while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
		spi_i2s_data_transmit(SPI2, 0); //spi2 tx (tx_pad with '0x00')
		while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
		rd.rxb = (u8) spi_i2s_data_receive(SPI2); //spi2 rx(1~n)
	}
	SPI_DM9051_CS_HIGH();
	return rd.rxb;
}
static void DM9051_Read_Mem(u8 *buf, u16 len)
{
	u16 i;
	SPI_DM9051_CS_LOW();
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, DM9051_MRCMD); //spi2 tx-cmd
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    /*rd.v0 = (u8)*/ spi_i2s_data_receive(SPI2); //spi2 rx (rx_pad)

	if (len & 1)
		len++;
	for(i=0; i<len; i++) {
		while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
		spi_i2s_data_transmit(SPI2, 0); //spi2 tx (tx_pad with '0x00')
		while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
		buf[i] = (u8) spi_i2s_data_receive(SPI2); //spi2 rx(1~n)
	}
	SPI_DM9051_CS_HIGH();
}
static void DM9051_Write_Mem(u8 *buf, u16 len)
{
	u16 i;
	SPI_DM9051_CS_LOW();
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, DM9051_MWCMD | 0x80); //spi2 tx-cmd
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    /*rd.v0 = (u8)*/ spi_i2s_data_receive(SPI2); //spi2 rx (rx_pad)

	if (len & 1)
		len++;
	for(i=0; i<len; i++) {
		while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
		spi_i2s_data_transmit(SPI2, buf[i]); //spi2 tx(1~n)
		while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
		/*rxpad = (u8)*/ spi_i2s_data_receive(SPI2); //spi2 rx (rx_pad)
	}
	SPI_DM9051_CS_HIGH();
}
uint16_t phy_read(uint16_t uReg)
{
	int w = 0;
	u16 uData;
	
	DM9051_Write_Reg(DM9051_EPAR, DM9051_PHY | uReg);
	DM9051_Write_Reg(DM9051_EPCR, 0xc);
	dm_delay_us(1);
	while(DM9051_Read_Reg(DM9051_EPCR) & 0x1) {
		dm_delay_us(1);
		if (++w >= 500) //5
			break;
	} //Wait complete
	
	DM9051_Write_Reg(DM9051_EPCR, 0x0);
	uData = (DM9051_Read_Reg(DM9051_EPDRH) << 8) | DM9051_Read_Reg(DM9051_EPDRL);
		
	return uData;
}
void phy_write(uint16_t reg, uint16_t value)
{
	int w = 0;

	DM9051_Write_Reg(DM9051_EPAR, DM9051_PHY | reg);
	DM9051_Write_Reg(DM9051_EPDRL, (value & 0xff));
	DM9051_Write_Reg(DM9051_EPDRH, ((value >> 8) & 0xff));
	/* Issue phyxcer write command */
	DM9051_Write_Reg(DM9051_EPCR, 0xa);
	dm_delay_us(1);
	while(DM9051_Read_Reg(DM9051_EPCR) & 0x1){
		dm_delay_us(1);
		if (++w >= 500) //5
			break;
	} //Wait complete

	DM9051_Write_Reg(DM9051_EPCR, 0x0);    
}

static
uint16_t eeprom_read(uint16_t uWord)
{
	int w = 0;
	u16 uData;
	
	DM9051_Write_Reg(DM9051_EPAR, uWord);
	DM9051_Write_Reg(DM9051_EPCR, 0x4);
	dm_delay_us(1);
	while(DM9051_Read_Reg(DM9051_EPCR) & 0x1) {
		dm_delay_us(1);
		if (++w >= 500) //5
			break;
	} //Wait complete
	
	DM9051_Write_Reg(DM9051_EPCR, 0x0);
	uData = (DM9051_Read_Reg(DM9051_EPDRH) << 8) | DM9051_Read_Reg(DM9051_EPDRL);
		
	return uData;
}

uint16_t dm9051_ethaddr_eepromread(uint8_t *buff)
{
	u16 uData;
	
	uData= eeprom_read(0);
	buff[1] = uData >> 8;
	buff[0] = uData & 0xff;
	uData= eeprom_read(1);
	buff[3] = uData >> 8;
	buff[2] = uData & 0xff;
	uData= eeprom_read(2);
	buff[5] = uData >> 8;
	buff[4] = uData & 0xff;
	return 0;
}

void dm9051_debug_flags(char *headstr)
{
	if (lw_flag & DM9051_FLAG_LINK_UP)
		link_reset = 1;

	if (headstr) {
		printf("%s: dm9051 driver lw_flag %s\r\n", headstr, 
				(lw_flag & DM9051_FLAG_LINK_UP) ? "linkup" : "linkdown");
		return;
	}
	printf("dm9051 driver lw_flag %s\n", (lw_flag & DM9051_FLAG_LINK_UP) ? "linkup" : "linkdown");
}
static void dm9051_update_flags(char *querystr, int linkup)
{
	//.if (linkup)
	//.	printf("%s: dm9051_lw - DM9051 linkup\r\n", querystr);
	//.else
	//.	printf("%s: dm9051_lw - DM9051 linkdown\r\n", querystr);
	
	if (linkup)
		dm9051_set_flags(lw_flag, DM9051_FLAG_LINK_UP);
	else
		dm9051_clear_flags(lw_flag, DM9051_FLAG_LINK_UP);

	dm9051_debug_flags(querystr);
}

/**
  * @brief  updates the link states
  * @param  headstr
  * @retval bmsr 7649: disconnect, 764d: connection, usually
  */
uint16_t dm9051_update_phyread(char *queryStr)
{
	uint16_t bmsr = phy_read(PHY_STATUS_REG); 
		
	if ((!dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP)) && (bmsr & PHY_LINKED_BIT))
		dm9051_update_flags(queryStr, bmsr & PHY_LINKED_BIT);
	else if (dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP) && (!(bmsr & PHY_LINKED_BIT)))
		dm9051_update_flags(queryStr, bmsr & PHY_LINKED_BIT);

	return bmsr;
}
	
static void dm9051_core_reset(void)
{
//u8 gpcr = DM9051_Read_Reg(DM9051_GPCR);
//DM9051_Write_Reg(DM9051_GPCR, gpcr | 0x01); //bit-0
	  DM9051_Write_Reg(DM9051_GPR, 0x00);		//Power on PHY
	  dm_delay_ms(1);
	  dm9051_clear_flags(lw_flag, DM9051_FLAG_LINK_UP);
	  unlink_count = 0;

	  DM9051_Write_Reg(DM9051_NCR, DM9051_REG_RESET); //iow(DM9051_NCR, NCR_RST);
	  DM9051_Write_Reg(DM9051_MBNDRY, MBNDRY_WORD); /* MemBound */
	  DM9051_Write_Reg(DM9051_PPCR, PPCR_PAUSE_COUNT); //iow(DM9051_PPCR, PPCR_SETTING); //#define PPCR_SETTING 0x08
	  DM9051_Write_Reg(DM9051_LMCR, LMCR_MODE1);
}

static void dm9051_show_rxbstatistic(u8 *htc, int n)
{
	int i, j;
	
	//.dm9051_chipid(); //Also [test][test][test].init
	printf("SHW rxbStatistic, 254 wrngs\r\n");
	for (i = 0 ; i < (n+2); i++) {
		if (!(i%32) && i) printf("\r\n");
		if (!(i%32) || !(i%16)) printf("%02x:", i);
		//if (!(i%16)) printf(" ");
		if (!(i%8)) printf(" ");
		if (i==0 || i==1) {
			printf("  ");
			continue;
		}
		j = i - 2;
		printf("%d ", htc[j]);
	}
	printf("\r\n");
}

void dm9051_mac_adr(uint8_t *macadd)
{
	dm9051_set_par(macadd);
	//show_par();
}
void dm9051_rx_mode(void)
{	
	dm9051_set_mar();
	//show_mar();
	
	dm9051_set_recv();
}

static void dm9051_set_par(u8 *macadd)
{
	int i;
	for (i=0; i<6; i++)
		DM9051_Write_Reg(DM9051_PAR+i, macadd[i]);
}
static void dm9051_set_mar(void)
{
	int i;
	for (i=0; i<8; i++)
		DM9051_Write_Reg(DM9051_MAR+i, (i == 7) ? 0x80 : 0x00);
}

static void dm9051_set_recv(void)
{
#if 0
	DM9051_Write_Reg(DM9051_FCR, FCR_DEFAULT); //iow(DM9051_FCR, FCR_FLOW_ENABLE);
	phy_write 04, flow
#endif
#if 1
	DM9051_Write_Reg(DM9051_IMR, IMR_PAR | IMR_PRM); //iow(DM9051_IMR, IMR_PAR | IMR_PTM | IMR_PRM);
	DM9051_Write_Reg(DM9051_RCR, RCR_DEFAULT | RCR_RXEN); //dm9051_fifo_RX_enable();
#endif
}

#define	DM9051_Read_Rxb	DM9051_Read_Mem2X

#define	TIMES_TO_RST	10

u8 ret_fire_time(u8 *histc, int csize, int i, u8 rxb)
{
	printf(" _dm9051f rxb %02x (times %2d)%c\r\n", rxb, histc[i], (histc[i]==2) ? '*': ' ');
	if (histc[i] >= TIMES_TO_RST) {
		dm9051_show_rxbstatistic(histc, csize);
		histc[i] = 1;
		return TIMES_TO_RST;
	}
	return 0;
}

static u16 err_rsthdlr(char *err_explain_str, u32 valuecode) //,int ret_cond //u8 zerochk
{
	//if (ret_cond)
	//	return 0;

	//if (zerochk && valuecode == 0)
	//	return 0;
		
	printf(err_explain_str, valuecode); //or "0x%02x"

	dm9051_core_reset(); //As: printf("rstc %d ,because rxb %02x (is %d times)\r\n", rstc, rxbyte, times);
  #if 1
	do { //[want more]
		//void tcpip_set_mac_address(void); //#include "netconf.h" (- For power on/off DM9051 demo-board, can restored working, Keep Lwip all no change.)
		//tcpip_set_mac_address(); //more for if user power-off the DM9051 chip, so instead of dm9051_set_recv();
		uint8_t macadd[6];
		lwip_get_mac_address(macadd);
		dm9051_mac_adr(macadd);
	} while(0);
  #endif
	dm9051_set_recv(); //of _dm9051_rx_mode();
	//dm9051_link_show(); //dm9051_show_link(); //_dm9051_show_timelinkstatistic(); //Add: show
	
	return 0;
}

/* if "expression" is true, then execute "handler" expression */
#define DM9051_RX_BREAK(expression, handler) do { if ((expression)) { \
  handler;}} while(0)
#define DM9051_TX_DELAY(expression, handler) do { if ((expression)) { \
  handler;}} while(0)

static u16 evaluate_link(void)
{
	if (unlink_count < UNLINK_COUNT_RESET) {
		unlink_count++; //if (unlink_count <= 2) //print(unlink_count);
	} else {
		if (link_reset) {
			link_reset = 0;
			DM9051_RX_BREAK((unlink_count >= UNLINK_COUNT_RESET), return err_rsthdlr("dm9051 rx moniter: %u\r\n", unlink_count));
		}
	}
	dm9051_update_phyread("rx"); //to dm9051_set_flags()/or dm9051_clear_flags()

	return 0;
}

static u16 evaluate_rxb(uint8_t rxb)
{
	int i;
	static u8 histc[254] = { 0 }; //static int rff_c = 0 ...;
	u8 times = 1;
	
	for (i = 0 ; i < sizeof(histc); i++) {
		if (rxb == (i+2)) {
			histc[i]++;
			times = ret_fire_time(histc, sizeof(histc), i, rxb);
			if (times == 0) //As: Hdlr (times : 0 or TIMES_TO_RST)
				return 0;
			return err_rsthdlr("_dm9051f rxb error accumunation times : %u\r\n", times); 
		}
	}
	return err_rsthdlr("dm9 impossible path error times : %u\r\n", times); //As: Hdlr (times : 1)
}

uint16_t dm9051_rx(uint8_t *buff)
{
	u8 rxbyte, rx_status;
	u8 ReceiveData[4];
	u16 rx_len;
	
	DM9051_RX_BREAK((!dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP)), return evaluate_link());
	
	rxbyte = DM9051_Read_Rxb(); //DM9051_Read_Reg(DM9051_MRCMDX);
	//DM9051_RXB_Basic(rxbyte); //(todo) Need sevice case.
	
	DM9051_RX_BREAK((rxbyte != 0x01 && rxbyte != 0), return evaluate_rxb(rxbyte));
	DM9051_RX_BREAK((rxbyte == 0), return 0);
		
	DM9051_Read_Mem(ReceiveData, 4);
	DM9051_Write_Reg(DM9051_ISR, 0x80);
	
	rx_status = ReceiveData[1];
	rx_len = ReceiveData[2] + (ReceiveData[3] << 8);
	
	DM9051_RX_BREAK((rx_status & 0xbf), return err_rsthdlr("_dm9051f rx_status error : 0x%02x\r\n", rx_status));
	DM9051_RX_BREAK((rx_len > PBUF_POOL_BUFSIZE), return err_rsthdlr("_dm9051f rx_len error : %u\r\n", rx_len));

	DM9051_Read_Mem(buff, rx_len);
	DM9051_Write_Reg(DM9051_ISR, 0x80);
	return rx_len;
}

void dm9051_tx(uint8_t *buf, uint16_t len)
{
	DM9051_Write_Reg(DM9051_TXPLL, len & 0xff);
	DM9051_Write_Reg(DM9051_TXPLH, (len >> 8) & 0xff);
	DM9051_Write_Mem(buf, len);
	DM9051_Write_Reg(DM9051_TCR, TCR_TXREQ); /* Cleared after TX complete */
	DM9051_TX_DELAY((DM9051_Read_Reg(DM9051_TCR) & TCR_TXREQ), dm_delay_us(5));
}

void dm9051_chipid(void)
{
	u16 chipid;
	u8 rev;

	chipid = (uint32_t)DM9051_Read_Reg(DM9051_PIDL) | (uint32_t)DM9051_Read_Reg(DM9051_PIDH) << 8;
	//[temp]
	if (chipid == 0x9000)
		chipid = 0x9051;
	if (chipid != 0x9051) {
		printf("DM9051 not found, chipid: %04x\r\n", chipid);
		printf("system stop\r\n");
		while(1);
	}
	printf("DM9051 found: %04x\r\n", chipid);
	
	rev = DM9051_Read_Reg(0x5c);
	printf("DM9051 chip rev: %02x\r\n", rev);
}

void dm9051_probe_link(void)
{
	int nsr_poll = 100;
	u8 val;
	
	do {
		val = DM9051_Read_Reg(DM9051_NSR) & 0x40;
		dm_delay_ms(10);
	} while (--nsr_poll && !val);
	
	dm9051_update_flags("probe_link", !!val);
	//dm9051_debug_flags("probe_link"); 
	//print(lw_flag);
}

void dm9051_init(uint8_t *adr)
{
#if 1
	lwip_set_mac_address(adr); // store in the driver.
#endif
	dm9051_chipid();
		
	dm9051_core_reset();
	dm9051_mac_adr(adr);
	dm9051_rx_mode();
	
	dm9051_probe_link();
}

static uint16_t bityes(uint8_t *hist) {
	uint16_t i;
	for (i = 0; i< 16; i++)
		if (hist[i])
			return 1;
	return 0;
}
static uint16_t link_show(void) {
	u8 n = 0, i, histnsr[16] = { 0, }, histlnk[16] = { 0, };
	u8 val;
	u16 lnk;
	u16 rwpa_w, mrr_rd;

	//.dm9051_chipid(); //Also [test][test][test].init
	do {
		//dm_delay_ms(100);
		n++;
		for (i= 0; i< 16; i++) {
			val = DM9051_Read_Reg(DM9051_NSR);
			lnk = dm9051_update_phyread("link_show");
			
			histnsr[i] += (val & 0x40) ? 1 : 0;
			histlnk[i] += (lnk & 0x04) ? 1 : 0;
		}
	} while(n < 20 && !bityes(histnsr) && !bityes(histlnk)); // 20 times for 2 seconds
	
	rwpa_w = (uint32_t)DM9051_Read_Reg(0x24) | (uint32_t)DM9051_Read_Reg(0x25) << 8; //DM9051_RWPAL
	mrr_rd = (uint32_t)DM9051_Read_Reg(0x74) | (uint32_t)DM9051_Read_Reg(0x75) << 8; //DM9051_MRRL;
	
	printf("(SHW timelink, 20 detects) det %d\r\n", n);
	
	/*for (i= 0; i< 16; i++)
		printf(" %s%d", (i==0) ? "check .nsr  " : (i==8) ? ".nsr.  " : "", histnsr[i]);
	printf(" %s\r\n", (bityes(histnsr) || bityes(histlnk)) ? "up" : "down");
	
	for (i= 0; i< 16; i++)
		printf(" %s%d", (i==0) ? "check .bmsr " : (i==8) ? ".bmsr. " : "", histlnk[i]);
	
	printf(" %s (rrp %x rwp %x)\r\n", (bityes(histnsr) || bityes(histlnk)) ? "up" : "down",
		mrr_rd, rwpa_w);*/
	
	for (i= 8; i< 16; i++)
		printf(" %s%d", (i==8) ? ".nsr " : "", histnsr[i]);
	for (i= 8; i< 16; i++)
		printf(" %s%d", (i==8) ? ".bmsr. " : "", histlnk[i]);
	
	printf(" (rrp %x rwp %x)\r\n", mrr_rd, rwpa_w);
	
	return bityes(histnsr) && bityes(histlnk);
}
uint16_t dm9051_link_show(void)
{
	return link_show();
}

#if 0
/*
 * In case to add more track rxbytes,
 * extend by add into histc[], and make it's counter to zeros in histc[].
 * Joseph, 20230409
 */
/*static u8 DM9051_ErrRXB(u8 rxbyte)
{
	const u8 hist[] = { 
		0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09, 0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19, 0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
		0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29, 0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
		0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39, 0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
		0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49, 0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
		0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59, 0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
		0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69, 0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
		0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79, 0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
		
		0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89, 0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
		0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99, 0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
		0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9, 0xaa,0xab,0xac,0xad,0xae,0xaf,
		0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9, 0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
		0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9, 0xca,0xcb,0xcc,0xcd,0xce,0xcf,
		0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9, 0xda,0xdb,0xdc,0xdd,0xde,0xdf,
		0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9, 0xea,0xeb,0xec,0xed,0xee,0xef,
		0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9, 0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
	};
	static u8 histc[254] = { 0 }; //static int rff_c = 0 ...;
	int i;
	
	for (i = 0 ; i < sizeof(hist); i++) {
		if (rxbyte == hist[i]) { //'0xff'..
			histc[i]++;
			//is in-histtab, will reset ( if rst_by_hist_check() break; )
			if (histc[i] >= TIMES_TO_RST) {
				if (histc[i] == TIMES_TO_RST) {
					printf(" _dm9051f rxb %02x (%d times)\r\n", rxbyte, histc[i]);
					dm9051_show_rxbstatistic(&histc[0], sizeof(hist));
					histc[i] = 1; //end-with-clean, wait increased..
					return TIMES_TO_RST;
				}
				return 0; //[Temp model] 
			}
			printf(" _dm9051f rxb %02x (%2d times)%c\r\n", rxbyte, histc[i],
					(histc[i]==2) ? '*': ' ');
			return 0;
		}
	}
	
	//not in-histtab, will reset ( rst_by_not_hist_check(); )
	printf(" _dm9051f rxb error ~~~ NOT MONITORED RXB ~~~ : %02x\r\n", rxbyte);
	return 1;
}*/
#endif

#if 0 //Original!

static void show_par(void) {
}

static void show_mar(void) {
}

/*
 * i.e. newp < oldp
 * and it has, (oldp >= 0x3a00 && newp < 0x1200)
 */
#define rule_oversize(oldp, newp) (oldp >= 0x3a00 && newp < 0x1200)
#define len_calc(oldp, newp) rule_oversize(oldp, newp) ? (newp + 0x4000 - 0xc00) - oldp : newp - oldp
static u8 DM9051_RXB_Error(u8 rxbyte);

static void info_xxxxxx(int rstc, u32 rxe, u32 rxc, u32 *rxec) {
}

static void DM9051_RXB_Basic(u8 rxbyte) // service state (state machine)
{
#if 0	
	static u8 service_state = 0; // flag: an Strange RXB to set, RXB 0x01 and/or RXB 0x00 to clean
#endif	
	return;
}

static int rstc = 0;

u16 _dm9051_rx(u8 *buff)
{
}

static int icount = 0;
static u16 rxmwr = 0x4000, rxmrd = 0x4000;

static void showdkind0(char *capt, int snum, /*u8 noused,*/ u8 rxb, u16 val_will_mrd, u16 len)
{
}

u16 showd(int kind, char *capt, int snum, u8 noused, u8 rxb, u16 memptr, u16 val_xon) {
}

static u8 DM9051_RXB_Error(u8 rxbyte)
{
}
#endif //Original!
