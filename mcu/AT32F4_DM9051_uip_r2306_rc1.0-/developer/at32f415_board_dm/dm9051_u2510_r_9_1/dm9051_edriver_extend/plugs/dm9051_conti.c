/**
 * @file     dm9051_conti.c
 * @brief    DM9051 Ethernet Controller Driver for uip, LWIP TCP/IP Stack
 * @version  V1.6.1
 * @date     2025-07-05
 *
 * @author   Joseph CHANG <joseph_chang@davicom.com.tw>
 * @copyright (c) 2023-2026 Davicom Semiconductor, Inc.
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
 * @note    First verification: AT32F403a
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
//#include <stdarg.h>
//#include <stdlib.h>

#include "core/dm9051.h"
extern const struct instance_data *dm9051_inf;

void tx_err_string(char *str)
{
  printf("%s\r\n", str);
}

void tx_create_head(uint8_t *th, uint16_t len)
{
  th[0] = len & 0xff;
  th[1] = (len >> 8) & 0xff;
  th[2] = 0x00;
  th[3] = 0x80;
}

static uint16_t get_tx_free(void)
{
  uint8_t rb = HAL_read_reg(DM9051_TXFSSR);
  return (uint16_t)rb * 64;
}

static int tx_free_enough0(uint16_t tx_tot)
{
  uint16_t tx_free = get_tx_free();
  return (tx_tot <= tx_free) ? 1 : 0;
}

uint16_t tx_free_poll_timeout(uint16_t tx_tot, uint16_t sleep_us, uint16_t timeout_us)
{
  for (;;)
  {
    if (tx_free_enough0(tx_tot))
      return tx_tot;
    if (!sleep_us)
      break;
    if (timeout_us < sleep_us)
      break;
    timeout_us -= sleep_us;
    ctick_delay_us(sleep_us);
  }
  return 0;
}

#if 1
void cspi_set_mode2_conti_rcr(void)
{
  //if (dm9051_driver_cfg->mode.tx_mode == FORCE_TX_CONTI_ON)
  //{}
  HAL_write_reg(DM9051_ATCR, ATCR_TX_MODE2);

  uint8_t val = RCR_DEFAULT | RCR_RXEN | RCR_WTDIS;
  HAL_write_reg(DM9051_RCR,
                (dm9051_inf->force_rcr_all) ? val | RCR_ALL | RCR_PRMSC: val); //TEST 'RCR_PRMSC'
}

void mem(uint16_t len)
{
  uint8_t th[TX_CM_HEADLEN];
  tx_create_head(th, len);
  HAL_write_mem(th, TX_CM_HEADLEN);
}

uint16_t cspi_tx_write_head(uint16_t len, uint16_t sleep_us, uint16_t timeout_us)
{
  //cspi_poll_free
  uint16_t tx_xxbst = ((len + 3) / 4) * 4;
  DM9051_TX_BREAK((!tx_free_poll_timeout(TX_CM_HEADLEN + tx_xxbst, sleep_us, timeout_us)),
                  tx_err_string("dm9051 tx -ENOMEM"), tx_xxbst = 0 /*return 0*/);
  if (tx_xxbst) {
	mem(len);
  }
  return tx_xxbst;
}

uint16_t cspi_tx_len_conti(uint16_t len)
{
	return cspi_tx_write_head(len, 1, 2500);
}

/* FORCE_TX_CONTI */
//void dm9051_single_tx_conti(uint8_t *buf, uint16_t len)
//{  
//  len = cspi_tx_len_conti(len);
//  if (len) {
//	cspi_tx_write(buf, len);
//	cspi_tx_req(buf, TCR_TXREQ);
//	while (HAL_read_reg(DM9051_TCR) & TCR_TXREQ) ;
//  }
//}
#endif
