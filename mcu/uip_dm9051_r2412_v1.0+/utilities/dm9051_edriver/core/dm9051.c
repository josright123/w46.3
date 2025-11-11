/**
 * @file     dm9051.c
 * @brief    DM9051 Ethernet Controller Driver for LWIP TCP/IP Stack
 * @version  V3.0
 * @date     2023-05-28
 * 
 * @author   Joseph CHANG <joseph_chang@davicom.com.tw>
 * @copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * 
 * @details
 *   This driver implements the hardware abstraction layer for the DM9051
 *   Ethernet controller, providing initialization, transmit, and receive
 *   functionality integrated with the LWIP TCP/IP stack.
 * 
 *   Key Features:
 *   - Full integration with uip and LWIP TCP/IP stack
 *   - Support for interrupt and polling modes
 *   - Configurable MAC address handling
 *   - Error detection and recovery mechanisms
 * 
 * @note    First verification: AT32F415
 */

/* Standard Library Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Project Specific Includes */
#include "platform_info/control/drv_control/conf_core.h"
#include "platform_info/control/drv_control/dm9051_drv_debug.h"

/*-----------------------------------------------------------------------------
 * Configuration and Definitions
 *-----------------------------------------------------------------------------*/

/* Buffer Size Definitions */
#define PBUF_POOL_BUFSIZE         (1514 + 4)    /* Maximum Ethernet frame size + header */
#define TIMES_TO_RST              10

/*-----------------------------------------------------------------------------
 * Function Prototypes
 *-----------------------------------------------------------------------------*/

/* Hardware Interface Functions */
uint8_t   cspi_read_reg(uint8_t reg);
void      cspi_write_reg(uint8_t reg, uint8_t val);
void      cspi_read_regs(uint8_t reg, uint8_t *buf, uint16_t len, csmode_t csmode);
void      cspi_write_regs(uint8_t reg, const uint8_t *buf, uint16_t len);
uint16_t  cspi_phy_read(uint16_t uReg);
void      cspi_phy_write(uint16_t reg, uint16_t value);
uint8_t   cspi_read_rxb(void);
void      cspi_tx_req(void);
void      cspi_read_mem(uint8_t *buf, uint16_t len);
void      cspi_write_mem(uint8_t *buf, uint16_t len);
void      ctick_delay_us(uint32_t nus);
void      ctick_delay_ms(uint16_t nms);

/* Internal Function Prototypes */
static uint16_t      impl_dm9051_rx(uint8_t *buff);
static const uint8_t *impl_dm9051_init(const uint8_t *adr);
static void          cspi_core_reset(void);
static const uint8_t *cspi_dm_start1(const uint8_t *adr);
static void          cspi_set_par(const uint8_t *adr);

/*-----------------------------------------------------------------------------
 * Public Interface Functions
 *-----------------------------------------------------------------------------*/

/**
 * @brief  Initialize DM9051 Ethernet controller
 * 
 * @param  adr   Pointer to MAC address array
 * @return       Pointer to configured MAC address
 * 
 * @note   This function performs complete initialization of the DM9051 including
 *         MAC address configuration and basic setup
 */
const uint8_t *dm9051_init(const uint8_t *adr) 
{
  const uint8_t *mac;
  
  mac = impl_dm9051_init(adr);

  printf("%s\r\n\r\n", mac ? "[ --- dm9051 found ok --- ] init done" : "[ --- dm9051 not found --- ]");
  return mac;
}

/**
 * @brief  Receive packet from DM9051
 * 
 * @param  buff  Buffer to store received packet
 * @return       Length of received packet or 0 if no packet
 * 
 * @note   This function handles the complete receive process including
 *         error checking and buffer management
 */
uint16_t dm9051_rx(uint8_t *buff) 
{
  uint16_t rx_len;
  
  rx_len = impl_dm9051_rx(buff);
  return rx_len;
}

/**
 * @brief  Transmit packet through DM9051
 * 
 * @param  buf   Buffer containing packet to transmit
 * @param  len   Length of packet
 * 
 * @note   This function handles the complete transmit process
 */
void dm9051_tx(uint8_t *buf, uint16_t len) 
{
  int us = 0;
  cspi_tx_write(buf, len);
  cspi_tx_req();
    //DM9051_TX_DELAY((cspi_read_reg(DM9051_TCR) & TCR_TXREQ), ctick_delay_us(5));
  do {
	  ctick_delay_us(5);
	  us += 5;
  } while((us < 2000) && (cspi_read_reg(DM9051_TCR) & TCR_TXREQ));
}

/**
 * @brief  Enable interrupts and read/write ISR register
 * @return Combined ISR status (upper 8 bits always 0xFF)
 */
uint16_t dm9051_isr_enab(void) 
{
  uint16_t isrs;
  
  isrs = cspi_read_reg(DM9051_ISR);
  cspi_write_reg(DM9051_ISR, (uint8_t)isrs);

  identify_irq_stat(ISTAT_IRQ_NOW2END);
  return isrs | (0xff << 8);
}

/**
 * @brief  Read DM9051 chip identification
 * @return 16-bit chip ID (PIDL | PIDH << 8)
 */
uint16_t cspi_read_chip_id(void) 
{
  uint8_t buff[2];
  
  cspi_read_regs(DM9051_PIDL, buff, 2, CS_EACH);
  return buff[0] | buff[1] << 8;
}

/**
 * @brief  Read control and status registers
 * @return 16-bit status (NCR | NSR << 8)
 */
uint16_t cspi_read_control_status(void) 
{
  uint8_t buff[2];
  
  cspi_read_regs(DM9051_NCR, buff, 2, CS_EACH);
  return buff[0] | buff[1] << 8;
}

/**
 * @brief  Read RX write pointer and memory data read address
 * 
 * @param  rwpa_wt  Pointer to store write pointer
 * @param  mdra_rd  Pointer to store read address
 */
void dm9051_read_rx_pointers(uint16_t *rwpa_wt, uint16_t *mdra_rd) 
{
  *rwpa_wt = (uint16_t)cspi_read_reg(0x24) |      /* DM9051_RWPAL */
             (uint16_t)cspi_read_reg(0x25) << 8;   /* DM9051_RWPAH */
             
  *mdra_rd = (uint16_t)cspi_read_reg(0x74) |      /* DM9051_MRRL */
             (uint16_t)cspi_read_reg(0x75) << 8;   /* DM9051_MRRH */
}

/**
 * @brief  Read comprehensive register information
 * 
 * @param  stat  Buffer to store status information (6 bytes)
 * 
 * @note   Combines chip ID, control status, and PHY status
 */
void dm9051_read_regs_info(uint8_t *stat) 
{
  uint16_t cs;
  uint32_t pbm;

  pbm = cspi_phy_read(PHY_STATUS_REG);
  pbm |= cspi_read_chip_id() << 16;
  cs = cspi_read_control_status();

  stat[0] = cs & 0xff;
  stat[1] = (cs >> 8) & 0xff;
  stat[2] = (pbm >> 24) & 0xff;
  stat[3] = (pbm >> 16) & 0xff;
  stat[4] = (pbm >> 8) & 0xff;
  stat[5] = pbm & 0xff;
}

/*-----------------------------------------------------------------------------
 * Hardware Configuration Functions
 *-----------------------------------------------------------------------------*/

/**
 * @brief  Power on PHY and wait for stabilization
 * 
 * @param  nms  Delay time in milliseconds
 */
void cspi_phycore_on(uint16_t nms) 
{
  cspi_write_reg(DM9051_GPR, 0x00);  /* Power on PHY */
  ctick_delay_ms(nms);
}

/**
 * @brief  Perform NCR reset and wait for completion
 * 
 * @param  nms  Delay time in milliseconds
 */
void cspi_ncr_reset(uint16_t nms) 
{
  cspi_write_reg(DM9051_NCR, DM9051_NCR_RESET);
  ctick_delay_ms(nms);
}

/**
 * @brief  Configure default software settings
 */
void cspi_soft_default(void) 
{
  uint8_t val = MBNDRY_WORD;  /* 16-bit */
  
  cspi_write_reg(DM9051_MBNDRY, val);
  cspi_write_reg(DM9051_PPCR, PPCR_PAUSE_COUNT);
  cspi_write_reg(DM9051_LMCR, LMCR_MODE1);
  cspi_write_reg(DM9051_INTR, INTR_ACTIVE_LOW);

#ifdef FORCE_CHKSUM_OFFLOAD
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  /* CHECKSUM_GEN_IP */
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  /* CHECKSUM_GEN_UDP */
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  /* CHECKSUM_GEN_TCP */  //(1 << 2) | (1 << 1) | (1 << 0)
  cspi_write_reg(DM9051_CSCR, TCSCR_UDPCS_ENABLE | TCSCR_TCPCS_ENABLE | TCSCR_IPCS_ENABLE);
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  /* CHECKSUM_CHECK_IP */
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  /* CHECKSUM_CHECK_UDP */
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  /* CHECKSUM_CHECK_TCP */  //(1 << 1) | (1 << 0)
  cspi_write_reg(DM9051_RCSSR, RCSSR_RCSEN | RCSSR_DCSE);
#endif
  identify_irq_stat(ISTAT_LOW_ACTIVE);
  trace_irq_stat(ISTAT_LOW_ACTIVE);
}

/**
 * @brief  Configure physical address registers
 * 
 * @param  adr  MAC address array
 */
static void cspi_set_par(const uint8_t *adr) 
{
  cspi_write_regs(DM9051_PAR, adr, 6);
}

/**
 * @brief  Configure multicast address registers
 */
void cspi_set_mar(void) 
{
  int i;
  
  for (i = 0; i < 8; i++) {
    cspi_write_reg(DM9051_MAR + i, (i == 7) ? 0x80 : 0x00);
  }
}

/**
 * @brief  Configure receive settings and interrupts
 */
void cspi_set_recv(void) 
{
#if 0
  cspi_write_reg(DM9051_FCR, FCR_DEFAULT);  /* iow(FCR_FLOW_ENABLE) */
  phy_write 04, flow
#endif

  /* Configure interrupt settings */
  identify_irq_stat(ISTAT_DM_IMR);
  trace_irq_stat(ISTAT_DM_IMR);
  cspi_write_reg(DM9051_IMR, IMR_PAR | IMR_PRM);  /* iow(IMR, IMR_PAR | IMR_PTM | IMR_PRM) */

  /* Configure receive settings */
  identify_irq_stat(ISTAT_DM_RCR);
  trace_irq_stat(ISTAT_DM_RCR);
  cspi_write_reg(DM9051_RCR, RCR_DEFAULT | RCR_RXEN);
}

/**
 * @brief  Read receive packet header
 * 
 * @param  receivedata  Buffer to store header data (4 bytes)
 */
void cspi_rx_head(uint8_t *receivedata) 
{
  cspi_read_mem(receivedata, 4);
  cspi_write_reg(DM9051_ISR, 0x80);
}

/**
 * @brief  Read receive packet data
 * 
 * @param  buff    Buffer to store packet data
 * @param  rx_len  Length of data to read
 */
void cspi_rx_read(uint8_t *buff, uint16_t rx_len) 
{
  cspi_read_mem(buff, rx_len);
  cspi_write_reg(DM9051_ISR, 0x80);
}

/**
 * @brief  Write transmit packet data
 * 
 * @param  buf  Buffer containing packet data
 * @param  len  Length of data to write
 */
void cspi_tx_write(uint8_t *buf, uint16_t len) 
{
  uint16_t pad = 0;  /* 8-bit */
  
  /* Set packet length */
  cspi_write_reg(DM9051_TXPLL, len & 0xff);
  cspi_write_reg(DM9051_TXPLH, (len >> 8) & 0xff);
  
  /* Write packet data with padding if needed */
  pad = len & 1;                   /* 16-bit */
  cspi_write_mem(buf, len + pad);  /* 8/16-bit */
}

/*-----------------------------------------------------------------------------
 * Environment and Initialization Functions
 *-----------------------------------------------------------------------------*/

/**
 * @brief  Initialize and verify DM9051 device
 * 
 * @param  id  Pointer to store chip ID
 * @return     1 if device found, 0 if not found
 */
int env_init_setup(uint16_t *id) 
{
  int ps = 0;
  uint32_t n = 0, heartbeat_stamp = dm9051_boards_heartbeat_now();
  uint8_t rev = cspi_read_reg(0x5c);
  
  /* Read and verify chip ID */
  *id = cspi_read_chip_id();
  if (*id == 0x9000) {
    *id = 0x9051;
  }

  if (*id == 0x9051) {
		printk("\r\n");
		printf("DM9051 found: %04x\r\n", *id);
  }
	else {
    printf("DM9051 not found, chipid: %04x\r\n", id);
	}

  /* Read and verify heartbeat */
  while (!n && ps++ < 3) {
	  n = 1000000; //1M times
	  while (n--)
		  dm9051_boards_heartbeat_now();

	  n = dm9051_boards_heartbeat_now() - heartbeat_stamp;
	  /* printkey("%d test, heartbeat %lu\r\n", ps, n); */
	  printf("%s: %lu, %s\r\n",
			 n ? "Heartbeat found" : "Heartbeat not found",
			 n, 
			 n == 1 ? "heartbeat increase type" :
			 n ? "heartbeat OK" :
				"heartbeat for delay function not exist fail");
  }

  //return (*id == 0x9051) ? 1 : 0;
  return (*id == 0x9051) && n ? 1 : 0;
}

/**
 * @brief  Display RX buffer statistics
 * 
 * @param  htc  History counter array
 * @param  n    Size of array
 */
static void dm9051_show_rxbstatistic(uint8_t *htc, int n) 
{
  int i;
#if (drv_print && PRINT_SEMA == SEMA_ON)
  int j;
#endif

  printf("SHW rxbStatistic, 254 wrngs\r\n");
  
  for (i = 0; i < (n + 2); i++) {
    if (!(i % 32) && i)
      printf("\r\n");
    if (!(i % 32) || !(i % 16))
      printf("%02x:", i);
    if (!(i % 8))
      printf(" ");
    if (i == 0 || i == 1) {
      printf("  ");
      continue;
    }
    
#if (drv_print && PRINT_SEMA == SEMA_ON)
    j = i - 2;
    printf("%d ", htc[j]);
#endif
  }
  printf("\r\n");
}

/**
 * @brief  Process RX buffer fire time
 * 
 * @param  histc   History counter array
 * @param  csize   Size of array
 * @param  i       Current index
 * @param  rxb     RX buffer value
 * @return         TIMES_TO_RST if reset needed, 0 otherwise
 */
static uint8_t ret_fire_time(uint8_t *histc, int csize, int i, uint8_t rxb) 
{
  printf(" _dm9051f rxb %02x (times %2d)%c\r\n", 
         rxb, histc[i], 
         (histc[i] == 2) ? '*' : ' ');
         
  if (histc[i] >= TIMES_TO_RST) {
    dm9051_show_rxbstatistic(histc, csize);
    histc[i] = 1;
    return TIMES_TO_RST;
  }
  
  return 0;
}

/**
 * @brief  Evaluate RX buffer status and handle errors
 * 
 * @param  rxb  RX buffer value to evaluate
 * @return      0 if successful, error code otherwise
 */
uint16_t env_evaluate_rxb(uint8_t rxb) 
{
  int i;
  static uint8_t histc[254] = {0};
  uint8_t times = 1;

  for (i = 0; i < sizeof(histc); i++) {
    if (rxb == (i + 2)) {
      histc[i]++;
      times = ret_fire_time(histc, sizeof(histc), i, rxb);
      
      if (times == 0)
        return 0;
        
      return env_err_rsthdlr("_dm9051f rxb error accumunation times : %u\r\n", times);
    }
  }
  
  return env_err_rsthdlr3("dm9 impossible path error times : %u\r\n", times);
}

/**
 * @brief  Process device reset with MAC address restoration
 * 
 * @param  macaddr  MAC address to restore
 * @return          Pointer to configured MAC address
 */
static const uint8_t *env_reset_process(const uint8_t *macaddr) 
{
  cspi_core_reset();
  return cspi_dm_start1(macaddr);
}

/**
 * @brief  Error callback handler
 * 
 * @param  explain_str  Error description string
 * @param  err_code     Error code
 */
void err_callback(char *explain_str, uint32_t err_code) 
{
  char bff[180];
  
  sprintf(bff, explain_str, err_code);
  printf("%s", bff);
}

/**
 * @brief  Error handler with reset capability
 * 
 * @param  err_explain_str  Error description format string
 * @param  valuecode        Error value
 * @return                  0 after reset completion
 */
uint16_t env_err_rsthdlr(char *err_explain_str, uint32_t valuecode) 
{
  char bff[180];
  
  sprintf(bff, err_explain_str, valuecode);
  printf("%s", bff);

  env_reset_process(identified_eth_mac());
  return 0;
}

/**
 * @brief  Error handler with callback and reset
 * 
 * @param  callback      Error callback function
 * @param  explain_str   Error description string
 * @param  code         Error code
 * @return              0 after reset completion
 */
uint16_t env_err_rsthdlr1(void (*callback)(char *, uint32_t), 
                          char *explain_str, 
                          uint32_t code) 
{
  callback(explain_str, code);
  env_reset_process(identified_eth_mac());
  return 0;
}

/**
 * @brief  Variable argument error handler with reset
 * 
 * @param  format   Error format string
 * @param  ...      Variable arguments
 * @return          0 after reset completion
 */
uint16_t env_err_rsthdlr3(const char *format, ...) 
{
  char bff[180];
  int val;
  va_list args;
  
  va_start(args, format);
  val = va_arg(args, int);
  sprintf(bff, format, val);
  printf("%s", bff);
  va_end(args);
  
  env_reset_process(identified_eth_mac());
  return 0;
}

/*-----------------------------------------------------------------------------
 * Core Implementation Functions
 *-----------------------------------------------------------------------------*/

/**
 * @brief  Core receive implementation
 * 
 * @param  buff  Buffer for received data
 * @return       Length of received packet or 0 if error
 */
static uint16_t impl_dm9051_rx(uint8_t *buff) 
{
  uint8_t rxbyte, rx_status;
  uint8_t ReceiveData[4];
  uint16_t rx_len, pad = 0;

  /* Read and validate RX byte */
  rxbyte = cspi_read_rxb();
  DM9051_RX_BREAK((rxbyte != 0x01 && rxbyte != 0),
                  return env_evaluate_rxb(rxbyte));
  DM9051_RX_BREAK((rxbyte == 0),
                  return 0);
  
  /* Read packet header */
  cspi_rx_head(ReceiveData);
  rx_status = ReceiveData[1];
  rx_len = ReceiveData[2] + (ReceiveData[3] << 8);
  
  /* Validate packet status and length */
  DM9051_RX_BREAK((rx_status & (0xbf & ~RSR_PLE)),
                  return env_err_rsthdlr1(err_callback, 
                                        "_dm9051f rx_status error : 0x%02x\r\n", 
                                        rx_status));
  DM9051_RX_BREAK((rx_len > PBUF_POOL_BUFSIZE), 
                  return env_err_rsthdlr("_dm9051f rx_len error : %u\r\n", 
                                       rx_len));

  /* Read packet data */
  pad = rx_len & 1;
  cspi_rx_read(buff, rx_len + pad);
  return rx_len;
}

/**
 * @brief  Core initialization implementation
 * 
 * @param  adr  MAC address to configure
 * @return      Pointer to configured MAC address
 */
static const uint8_t *impl_dm9051_init(const uint8_t *adr) 
{
  uint16_t id;
  const uint8_t *mac = NULL;

  /* Initialize and verify device */
  if (!env_init_setup(&id)) {
#ifdef FORCE_STOP_IF_DM9051_NOT_FOUND
    printf("system stop\r\n");
    while (1);
#else
	return NULL;
#endif
  }

  /* Configure MAC address */
  mac = identify_eth_mac(adr);
  trace_identify_eth_mac();
  return env_reset_process(mac);
}

/**
 * @brief  Core reset implementation
 */
static void cspi_core_reset(void) 
{
  cspi_ncr_reset(5);
  cspi_phycore_on(25);
  cspi_soft_default();
}

/**
 * @brief  Start DM9051 with MAC configuration
 * 
 * @param  adr  MAC address to configure
 * @return      Pointer to configured MAC address
 */
static const uint8_t *cspi_dm_start1(const uint8_t *adr) 
{
//#ifdef ETHERNET_INTERRUPT_MODE
//#endif
  if (dm_init_info.interrrpt_mode) {
	cint_enable_mcu_irq();
  }

  /* Configure MAC and multicast addresses */
  cspi_set_par(adr);
  cspi_set_mar();
  
  /* Initialize receive settings */
  cspi_set_recv();
  
  return adr;
}

/* Hardware Interface Functions */
uint8_t   cspi_read_reg(uint8_t reg);
void      cspi_write_reg(uint8_t reg, uint8_t val);
void      ctick_delay_us(uint32_t nus);

// Additional Functions for Register Operations
void cspi_read_regs_long(uint8_t reg, uint8_t *buf, uint16_t len);
void cspi_read_regs_each(uint8_t reg, uint8_t *buf, uint16_t len);

void cspi_read_regs(uint8_t reg, uint8_t *buf, uint16_t len, csmode_t csmode) {
    if (csmode == CS_LONG) {
        cspi_read_regs_long(reg, buf, len);
    } else { // CS_EACH
        cspi_read_regs_each(reg, buf, len);
    }
}

void cspi_write_regs(uint8_t reg, const uint8_t *buf, uint16_t len)
{
	uint16_t i;
    for (i = 0; i < len; i++, reg++) {
        cspi_write_reg(reg, buf[i]);
    }
}

/*******************************************************************************
 * PHY Access Functions
 ******************************************************************************/
uint16_t cspi_phy_read(uint16_t uReg)
{
    int w = 0;
    uint16_t uData;

    cspi_write_reg(DM9051_EPAR, DM9051_PHY | uReg);
    cspi_write_reg(DM9051_EPCR, 0xc);
    ctick_delay_us(1);
    
    while (cspi_read_reg(DM9051_EPCR) & 0x1) {
        ctick_delay_us(1);
        if (++w >= 500) break; // Timeout
    }

    cspi_write_reg(DM9051_EPCR, 0x0);
    uData = (cspi_read_reg(DM9051_EPDRH) << 8) | cspi_read_reg(DM9051_EPDRL);
    return uData;
}

void cspi_phy_write(uint16_t reg, uint16_t value)
{
    int w = 0;

    cspi_write_reg(DM9051_EPAR, DM9051_PHY | reg);
    cspi_write_reg(DM9051_EPDRL, (value & 0xff));
    cspi_write_reg(DM9051_EPDRH, ((value >> 8) & 0xff));
    cspi_write_reg(DM9051_EPCR, 0xa);
    ctick_delay_us(1);
    
    while (cspi_read_reg(DM9051_EPCR) & 0x1) {
        ctick_delay_us(1);
        if (++w >= 500) break; // Timeout
    }

    cspi_write_reg(DM9051_EPCR, 0x0);
}

// TX Request Function
void cspi_tx_req(void) {
    cspi_write_reg(DM9051_TCR, TCR_TXREQ); // Cleared after TX complete
}

// Delay Function
void ctick_delay_us(uint32_t nus)
{
    uint32_t start = dm9051_boards_heartbeat_now();
    while ((dm9051_boards_heartbeat_now() - start) < ((nus + 999) / 1000));
}

void ctick_delay_ms(uint16_t nms)
{
    uint32_t start = dm9051_boards_heartbeat_now();
    while ((dm9051_boards_heartbeat_now() - start) < nms);
}
