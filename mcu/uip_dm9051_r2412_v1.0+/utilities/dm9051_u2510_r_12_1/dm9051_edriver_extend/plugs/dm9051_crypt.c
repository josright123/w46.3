/**
 * @file     dm9051_crypt.c
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
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>

//#include "core/ver.h"
#include "core/dm9051.h"

static uint8_t rsv_word;

void cspi_setup_buswork(void)
{
  /* read_crypt_word */
  uint8_t crypt_1 = HAL_read_reg(DM9051_PIDL);
  uint8_t crypt_2 = HAL_read_reg(DM9051_PIDH);
  HAL_write_reg(0x49, crypt_1);
  HAL_write_reg(0x49, crypt_2);
  rsv_word = HAL_read_reg(0x49);
}

//static
void bus_work(uint8_t *buff, uint16_t crlen)
{
  uint16_t j;
  for (j = 0; j < crlen; j++)
  {
    buff[j] ^= rsv_word;
  }
}
