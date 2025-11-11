/**
  **************************************************************************
  * @file     iperf.h
  * @brief    iperf tool header
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#ifndef __IPERF_H__
#define __IPERF_H__

#include "lwip/opt.h"
#include "lwip/ip_addr.h"

#ifdef __cplusplus
 extern "C" {
#endif

/** lwIPerf test results */
enum lwiperf_report_type
{
  /** The server side test is done */
  LWIPERF_TCP_DONE_SERVER,
  /** The client side test is done */
  LWIPERF_TCP_DONE_CLIENT,
  /** Local error lead to test abort */
  LWIPERF_TCP_ABORTED_LOCAL,
  /** Data check error lead to test abort */
  LWIPERF_TCP_ABORTED_LOCAL_DATAERROR,
  /** Transmit error lead to test abort */
  LWIPERF_TCP_ABORTED_LOCAL_TXERROR,
  /** Remote side aborted the test */
  LWIPERF_TCP_ABORTED_REMOTE
};

/** Prototype of a report function that is called when a session is finished.
    This report function can show the test results.
    @param report_type contains the test result */
typedef void (*lwiperf_report_fn)(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec);

void iperf_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __IPERF_H__ */
