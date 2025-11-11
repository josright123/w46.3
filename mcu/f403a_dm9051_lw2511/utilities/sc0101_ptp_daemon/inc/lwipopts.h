/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_LWIPOPTS_H
#define LWIP_HDR_LWIPOPTS_H

//#define LWIP_TESTMODE                   0

#define LWIP_IPV4                         1
//#define LWIP_TIMERS                     0

//#define LWIP_CHECKSUM_ON_COPY           1
//#define TCP_CHECKSUM_ON_COPY_SANITY_CHECK 1
//#define TCP_CHECKSUM_ON_COPY_SANITY_CHECK_FAIL(printfmsg) LWIP_ASSERT("TCP_CHECKSUM_ON_COPY_SANITY_CHECK_FAIL", 0)

/* We link to special sys_arch.c (for basic non-waiting API layers unit tests) */
#define NO_SYS                           1 //0 //1
#define SYS_LIGHTWEIGHT_PROT             0
#define LWIP_NETCONN                     !NO_SYS
#define LWIP_SOCKET                      !NO_SYS
#define LWIP_NETCONN_FULLDUPLEX          LWIP_SOCKET
#define LWIP_NETBUF_RECVINFO             1
#define LWIP_HAVE_LOOPIF                 1
#define LWIP_NETIF_LINK_CALLBACK         1
//#define TCPIP_THREAD_TEST

/* Enable DHCP to test it, disable UDP checksum to easier inject packets */
#define LWIP_DHCP                       1 //1 //0 //1 //1 //0 //1 //0
#define EDRIVER_ADDING_PTP				1 //0	//1
#define LWIP_PTP                        1 //0 // 1 //1
#define IP_REASSEMBLY                   0 //[jj]
#define LWIP_NO_CTYPE_H					1 //[jj.20250729]
#define OPTIONAL_SYSTICK_CORE_CM4		FALSE //[JJ.20250820]
//#define LWIP_PROVIDE_ERRNO				  //[20250826]
#define LWIPERF_APP						1
#define LWIP_MQTT						1 //0 //1 //1 //[20250828]
//#define LWIP_PHY_DOWN_MEASUREMENT		1 //[20250905]
//#define LWIP_PHY_DOWN_SECONDS			50 //[20250905]
#if 1
//[Joseph] use debug config
//#define MQTT_DEBUG              		LWIP_DBG_ON //[20250911] //| LWIP_DBG_HALT //LWIP_DBG_OFF
//#define IP_DEBUG                      LWIP_DBG_ON //[20250911]
//#define TCP_INPUT_DEBUG                 LWIP_DBG_ON //[20250911]
//#define DHCP_DEBUG                      LWIP_DBG_ON //[20250919]
#endif
#define SLAVE_MAKE_DELAY_REQ_FAVOR_TS	1 //[prepare 250916]
#if LWIPERF_APP
//#define TCP_TMR_INTERVAL       		10  //[20251003] /* 250, The TCP timer interval in milliseconds. */
#define TCP_TMR_INTERVAL       			80  //[20251027] /* 250, The TCP timer interval in milliseconds. */
											//change larger because, api_lib.c(211): warning:  #69-D: integer conversion resulted in truncation
#endif

/**
 * MQTT 變數標頭緩衝區長度
 * 必須至少為最長 incoming topic 的大小 + 8
 * 若要避免分片的 incoming publish，設定為 max topic length + max payload + 8
#define MQTT_VAR_HEADER_BUFFER_LEN 128
#define MQTT_VAR_HEADER_BUFFER_LEN 256
#define MQTT_VAR_HEADER_BUFFER_LEN 658
/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/offset (50)
 */
#define MQTT_VAR_HEADER_BUFFER_LEN				(50 + 600 + 8)

/**
 * 所有的回調函數
 * Used in "mqtt_client_cb.c"
 * For the subcribe imcoming data size
#define MQTT_SUBSCRIBE_PAYLOAD_BUFFER_SIZE 256
 */
#define MQTT_SUBSCRIBE_PAYLOAD_BUFFER_SIZE		560

/* Minimal changes to opt.h required for tcp unit tests: */
/* TCP Maximum segment size. */

#define TCP_MSS                          (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4
#define MEM_SIZE                         (32*1024)
#define TCP_SND_QUEUELEN                 (6 * TCP_SND_BUF)/TCP_MSS
#define MEMP_NUM_TCP_SEG                 TCP_SND_QUEUELEN
#define TCP_SND_BUF                      (2 * TCP_MSS)
#define TCP_WND                          (2 * TCP_MSS)
#define LWIP_WND_SCALE                   0
#define TCP_RCV_SCALE                    0
#define PBUF_POOL_SIZE                   10 /* pbuf tests need ~200KByte */
/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE                2000
//#define TCP_QUEUE_OOSEQ         		 0

/* ---------- TCP options ---------- */
#define LWIP_TCP                         1
#define TCP_TTL                          255

/* Enable IGMP and MDNS for MDNS tests */
#define LWIP_IGMP                        1
#define LWIP_MDNS_RESPONDER              1
#define LWIP_NUM_NETIF_CLIENT_DATA       (LWIP_MDNS_RESPONDER)

/* Minimal changes to opt.h required for etharp unit tests: */
#define ETHARP_SUPPORT_STATIC_ENTRIES    1

#define MEMP_NUM_SYS_TIMEOUT             (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 8)

/* MIB2 stats are required to check IPv4 reassembly results */
#define MIB2_STATS                       1

/* netif tests want to test this, so enable: */
//#define LWIP_NETIF_EXT_STATUS_CALLBACK  1

/* Check lwip_stats.mem.illegal instead of asserting */
#define LWIP_MEM_ILLEGAL_FREE(msg)       /* to nothing */

#define CHECKSUM_BY_HARDWARE 
#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0 
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif

//#define LWIP_NOASSERT //to be defined, fix to run ptpd (or define lwipopts.h for LWIP_PLATFORM_ASSERT)
//or #define LWIP_PLATFORM_ASSERT(x)    // 禁用 lwIP assert 輸出，防止鏈入 stdio_streams
//or #define LWIP_PLATFORM_DIAG(x)      // 禁用 lwIP debug 輸出，防止鏈入 stdio_streams

#endif /* LWIP_HDR_LWIPOPTS_H */
