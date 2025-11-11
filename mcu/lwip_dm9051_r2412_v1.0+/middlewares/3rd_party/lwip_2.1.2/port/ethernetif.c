/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp/pppoe.h"
#include "err.h"
//#include "lwip_ethif/nosys/ethernetif_types.h"
#include "ethernetif.h"
#include "netconf.h"

void low_level_init(struct netif *netif);
err_t low_level_output(struct netif *netif, struct pbuf *p);
struct pbuf *low_level_input(struct netif *netif);

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init_E() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init_E(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init_E: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);

  //netif->name[0] = AT_IFNAME0;
  //netif->name[1] = AT_IFNAME1;
 
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;

  if (NETIF_IS_EMAC(netif)) {
//	ethernetif->dm_hardware_ptp_ts = 0;
//	netif->state = ethernetif;
//    netif->linkoutput = low_level_output_E;
//	/* initialize the hardware */
//	low_level_init_E(netif);
  }
  else if (NETIF_IS_V51(netif)) {
	ethernetif->dm_hardware_ptp_ts = 1;
	netif->state = ethernetif;
    netif->linkoutput = low_level_output;
	/* initialize the hardware */
	low_level_init(netif);
  }

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
#ifdef LWIP_IGMP
  netif->flags |= NETIF_FLAG_IGMP | NETIF_FLAG_BROADCAST;
#endif

  return ERR_OK;
}

#if 0
#include "at32f435_437_emac.h"
#include <string.h>

//#include "platform_info/control/dm9051opts.h" //[ETHERNET_INTERRUPT_MODE/]

/* TCP and ARP timeouts */
volatile int tcp_end_time, arp_end_time;

/* Define those to better describe your network interface. */
//#define AT_IFNAME0 'a'
//#define AT_IFNAME1 't'

//#define ADJ_FREQ_BASE_ADDEND             0x33333333
#define ADJ_FREQ_BASE_ADDEND             0x33320123//0x33320000 //0x3323DC00	//modify 2023-11-15
#define ADJ_FREQ_BASE_INCREMENT          0x2B
#define EMAC_PTP_FINEUPDATE              ((uint32_t)0x00000001)  /*!< Fine Update method */
#define EMAC_PTP_COARSEUPDATE            ((uint32_t)0x00000000)  /*!< Coarse Update method */

#define EMAC_DMARxDesc_FrameLengthShift  16

/* Forward declarations. */
err_t  ethernetif_input_E(struct netif *netif);

#define EMAC_RXBUFNB        2
#define EMAC_TXBUFNB        2

uint8_t MACaddr[6];
#if LWIP_PTP
emac_dma_desc_type  DMAPTPRxDscrTab[EMAC_RXBUFNB], DMAPTPTxDscrTab[EMAC_TXBUFNB];/* Ethernet Rx & Tx PTP Helper Descriptors */
#endif
emac_dma_desc_type  DMARxDscrTab[EMAC_RXBUFNB], DMATxDscrTab[EMAC_TXBUFNB];/* Ethernet Rx & Tx DMA Descriptors */
uint8_t Rx_Buff[EMAC_RXBUFNB][EMAC_MAX_PACKET_LENGTH], Tx_Buff[EMAC_TXBUFNB][EMAC_MAX_PACKET_LENGTH];/* Ethernet buffers */

extern __IO emac_dma_desc_type  *dma_tx_desc_to_set;
extern __IO emac_dma_desc_type  *dma_rx_desc_to_get;
extern __IO emac_dma_desc_type  *ptp_dma_tx_desc_to_set;
extern __IO emac_dma_desc_type  *ptp_dma_rx_desc_to_get;

typedef struct{
    u32 length;
    u32 buffer;
    __IO emac_dma_desc_type *descriptor;
#if LWIP_PTP
    __IO emac_dma_desc_type *ptpdescriptor;
#endif
}FrameTypeDef;

FrameTypeDef emac_rxpkt_chainmode(void);
u32 emac_getcurrenttxbuffer(void);
error_status emac_txpkt_chainmode(u16 FrameLength);

#if LWIP_PTP
static u8 * emac_ptptxpkt_preparebuffer(void) {
  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((dma_tx_desc_to_set->status & EMAC_DMATXDESC_OWN) != (u32)RESET)
  {
    /* Return ERROR: OWN bit set */
    return NULL;
  }

  dma_tx_desc_to_set->buf1addr = ptp_dma_tx_desc_to_set->buf1addr;
  dma_tx_desc_to_set->buf2nextdescaddr = ptp_dma_tx_desc_to_set->buf2nextdescaddr;

  return (u8 *) emac_getcurrenttxbuffer();
}

static u32 emac_ptp_txpkt_chainmode(u16 FrameLength, struct ptptime_t * timestamp)
{
  uint32_t timeout = 0;
  /* Setting the Frame Length: bits[12:0] */
  dma_tx_desc_to_set->controlsize = (FrameLength & EMAC_DMATXDESC_TBS1);

  /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
  dma_tx_desc_to_set->status |= EMAC_DMATXDESC_LS | EMAC_DMATXDESC_FS;

  /* Set Own bit of the Tx descriptor status: gives the buffer back to ETHERNET DMA */
  dma_tx_desc_to_set->status |= EMAC_DMATXDESC_OWN;

  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if ((EMAC_DMA->sts & EMAC_DMA_TBU_FLAG) != (u32)RESET)
  {
    /* Clear TBUS ETHERNET DMA flag */
    EMAC_DMA->sts = EMAC_DMA_TBU_FLAG;
    /* Resume DMA transmission*/
    EMAC_DMA->tpd = 0;
  }

  do
  {
    timeout++;
  } while (!(dma_tx_desc_to_set->status & EMAC_DMATXDESC_TTSS) && (timeout < 0x0004FFFF));
  /* Return ERROR in case of timeout */
  if(timeout == 0x0004FFFF)
  {
    return ERROR;
  }
  
  timestamp->tv_nsec = emac_ptpsubsecond2nanosecond(dma_tx_desc_to_set->timestamp_l);
  timestamp->tv_sec = dma_tx_desc_to_set->timestamp_h;

  /* Clear the dma_tx_desc_to_set status register TTSS flag */
  dma_tx_desc_to_set->status &= ~EMAC_DMATXDESC_TTSS;

  dma_tx_desc_to_set->buf1addr = ptp_dma_tx_desc_to_set->buf1addr;
  dma_tx_desc_to_set->buf2nextdescaddr = ptp_dma_tx_desc_to_set->buf2nextdescaddr;

  /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */
  /* Chained Mode */
  /* Selects the next DMA Tx descriptor list for next buffer to send */
  dma_tx_desc_to_set = (emac_dma_desc_type*) (dma_tx_desc_to_set->buf2nextdescaddr);

  if(ptp_dma_tx_desc_to_set->status != 0)
  {
    ptp_dma_tx_desc_to_set = (emac_dma_desc_type*) (ptp_dma_tx_desc_to_set->status);
  }
  else
  {
    ptp_dma_tx_desc_to_set++;
  }

  /* Return SUCCESS */
  return SUCCESS;
}

static void emac_ptpstart(uint32_t updatemethod) {
  /* Mask the Time stamp trigger interrupt by setting bit 9 in the MACIMR register. */
  emac_interrupt_mask_set(EMAC_INTERRUPT_TST_MASK, TRUE);
  /* Program Time stamp register bit 0 to enable time stamping. */
  emac_ptp_timestamp_enable(TRUE);

  /* Program the Subsecond increment register based on the PTP clock frequency. */
  emac_ptp_subsecond_increment_set(ADJ_FREQ_BASE_INCREMENT); /* to achieve 20 ns accuracy, the value is ~ 43 */

  if (updatemethod == EMAC_PTP_FINEUPDATE) {

    /* If you are using the Fine correction method, program the Time stamp addend register
     * and set Time stamp control register bit 5 (addend register update). */
    emac_ptp_timestamp_addend_set(ADJ_FREQ_BASE_ADDEND);
    //emac_ptp_timestamp_system_time_update(TRUE);
    emac_ptp_addend_register_update(TRUE);
    /* Poll the Time stamp control register until bit 5 is cleared. */
    while(emac_ptp_flag_get(EMAC_PTP_ARU_FLAG));
    /* To select the Fine correction method (if required),
     * program Time stamp control register  bit 1. */
    emac_ptp_timestamp_fine_update_enable(TRUE);
  }
  else {
    /* To select the Fine correction method (if required),
     * program Time stamp control register  bit 1. */
    emac_ptp_timestamp_fine_update_enable(FALSE);
  }
  
  /* Program the Time stamp high update and Time stamp low update registers
   * with the appropriate time value. */
  emac_ptp_system_time_set(EMAC_PTP_POSITIVETIME, 0x7818BEF1, 0);
  /* Set Time stamp control register bit 2 (Time stamp init). */
  emac_ptp_timestamp_system_time_init(TRUE);
  while(emac_ptp_flag_get(EMAC_PTP_TI_FLAG));
  
#ifdef EMAC_USE_ENHANCED_DMA_DESCRIPTOR
  emac_dma_alternate_desc_size(TRUE);
#endif
  emac_ptp_snapshot_received_frames_enable(TRUE);
  /* The Time stamp counter starts operation as soon as it is initialized
   * with the value written in the Time stamp update register. */

  /* Enable the MAC receiver and transmitter for proper time stamping. ETH_Start(); */
}

static void emac_ptp_rxpkt_chainmode(FrameTypeDef * frame)
{
  u32 framelength = 0;
  frame->length = 0;
  frame->buffer = 0;

  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((dma_rx_desc_to_get->status & EMAC_DMARXDESC_OWN) != (u32)RESET)
  {
    frame->length = FALSE;

    if(emac_dma_flag_get(EMAC_DMA_RBU_FLAG))
    {
      /* Clear RBUS ETHERNET DMA flag */
      emac_dma_flag_clear(EMAC_DMA_RBU_FLAG);
      /* Resume DMA reception */
      EMAC_DMA->rpd_bit.rpd = FALSE;
    }
    /* Return error: OWN bit set */
    return;
  }

  if(((dma_rx_desc_to_get->status & EMAC_DMATXDESC_ES) == (u32)RESET) &&
     ((dma_rx_desc_to_get->status & EMAC_DMARXDESC_LS) != (u32)RESET) &&
     ((dma_rx_desc_to_get->status & EMAC_DMARXDESC_FS) != (u32)RESET))
  {
    /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
    framelength = ((dma_rx_desc_to_get->status & EMAC_DMARXDESC_FL) >> EMAC_DMARxDesc_FrameLengthShift) - 4;

    /* Get the addrees of the actual buffer */
    frame->buffer = ptp_dma_rx_desc_to_get->buf1addr;
  }
  else
  {    
    framelength = 0;
  }

  frame->length = framelength;

  frame->descriptor = dma_rx_desc_to_get;
  frame->ptpdescriptor = ptp_dma_rx_desc_to_get;

  /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */
  /* Chained Mode */
  /* Selects the next DMA Rx descriptor list for next buffer to read */
  dma_rx_desc_to_get = (emac_dma_desc_type*) (ptp_dma_rx_desc_to_get->buf2nextdescaddr);
  if(ptp_dma_rx_desc_to_get->status != 0)
  {
    ptp_dma_rx_desc_to_get = (emac_dma_desc_type*) (ptp_dma_rx_desc_to_get->status);
  }
  else
  {
    ptp_dma_rx_desc_to_get++;
  }
  
  return;
}

static void emac_ptprxpkt_chainmode_cleanup(FrameTypeDef * frame, struct ptptime_t * timestamp)
{
  timestamp->tv_nsec = emac_ptpsubsecond2nanosecond(frame->descriptor->timestamp_l);
  timestamp->tv_sec = frame->descriptor->timestamp_h;

  frame->descriptor->buf1addr = frame->ptpdescriptor->buf1addr;
  frame->descriptor->buf2nextdescaddr = frame->ptpdescriptor->buf2nextdescaddr;
}
#endif

/**
 * Setting the MAC address.
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
//void lwip_set_mac_address(uint8_t* macadd)
//{
//  MACaddr[0] = macadd[0];
//  MACaddr[1] = macadd[1];
//  MACaddr[2] = macadd[2];
//  MACaddr[3] = macadd[3];
//  MACaddr[4] = macadd[4];
//  MACaddr[5] = macadd[5];

//  emac_local_address_set(macadd);
//}


/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init_E().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static
void low_level_init_E(struct netif *netif)
{
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] =  MACaddr[0];
  netif->hwaddr[1] =  MACaddr[1];
  netif->hwaddr[2] =  MACaddr[2];
  netif->hwaddr[3] =  MACaddr[3];
  netif->hwaddr[4] =  MACaddr[4];
  netif->hwaddr[5] =  MACaddr[5];

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_PTP
  emac_dma_alternate_desc_size(TRUE);
  /* Initialize Tx Descriptors list: Chain Mode */
  emac_ptp_dma_descriptor_list_address_set(EMAC_DMA_TRANSMIT, DMATxDscrTab, DMAPTPTxDscrTab, &Tx_Buff[0][0], EMAC_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  emac_ptp_dma_descriptor_list_address_set(EMAC_DMA_RECEIVE, DMARxDscrTab, DMAPTPRxDscrTab, &Rx_Buff[0][0], EMAC_RXBUFNB);
#else
  /* Initialize Tx Descriptors list: Chain Mode */
  emac_dma_descriptor_list_address_set(EMAC_DMA_TRANSMIT, DMATxDscrTab, &Tx_Buff[0][0], EMAC_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  emac_dma_descriptor_list_address_set(EMAC_DMA_RECEIVE, DMARxDscrTab, &Rx_Buff[0][0], EMAC_RXBUFNB);
#endif

  /* Enable Ethernet Rx interrrupt */
  { int i;
    for(i=0; i < EMAC_RXBUFNB; i++)
    {
      emac_dma_rx_desc_interrupt_config(&DMARxDscrTab[i], TRUE);
    }
#ifdef CHECKSUM_BY_HARDWARE
    for(i=0; i < EMAC_TXBUFNB; i++)
    {
      DMATxDscrTab[i].status |= EMAC_DMATXDESC_CIC_TUI_FULL;
    }
#endif
#if LWIP_PTP
  /* Enable PTP Timestamping */
  emac_ptpstart(EMAC_PTP_FINEUPDATE);
  /* ETH_PTPStart(ETH_PTP_CoarseUpdate); */
#endif
  }

  /* Enable MAC and DMA transmission and reception */
  emac_start();

}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static
err_t low_level_output_E(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  int l = 0;
#if LWIP_PTP
  struct ptptime_t timestamp;
  u8 *buffer = emac_ptptxpkt_preparebuffer();
#else
  u8 *buffer =  (u8 *)emac_getcurrenttxbuffer();
#endif

  for(q = p; q != NULL; q = q->next)
  {
    memcpy((u8_t*)&buffer[l], q->payload, q->len);
    l = l + q->len;
  }
#if LWIP_PTP
  if(emac_ptp_txpkt_chainmode(l, &timestamp) == SUCCESS) /* JJ: E2E's T3 */
  {
    p->time_sec = timestamp.tv_sec;
    p->time_nsec = timestamp.tv_nsec;
  }
  else 
  {
    return ERR_IF;
  }
#else
  if(emac_txpkt_chainmode(l) == ERROR)
  {
    return ERR_MEM;
  }
#endif

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
//static 
struct pbuf *low_level_input(struct netif *netif)
{
  struct pbuf *p, *q;
  u16_t len;
  int l =0;
  FrameTypeDef frame;
  u8 *buffer;
#if LWIP_PTP
  struct ptptime_t timestamp;
#endif

  p = NULL;
#if LWIP_PTP
  frame.ptpdescriptor = NULL;
  emac_ptp_rxpkt_chainmode(&frame); /* JJ: E2E's (T1) */
#else
  frame = emac_rxpkt_chainmode();
#endif
  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = frame.length;

  buffer = (u8 *)frame.buffer;

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

  if (p != NULL)
  {
    for (q = p; q != NULL; q = q->next)
    {
      memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
      l = l + q->len;
    }
  }

#if LWIP_PTP
  emac_ptprxpkt_chainmode_cleanup(&frame, &timestamp); /* JJ: E2E's T1 */
  if(p != NULL)
  {
    p->time_sec = timestamp.tv_sec;
    p->time_nsec = timestamp.tv_nsec;
  }
#endif
  
  /* Set Own bit of the Rx descriptor Status: gives the buffer back to ETHERNET DMA */
  frame.descriptor->status |= EMAC_DMARXDESC_OWN;

  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
  if(emac_dma_flag_get(EMAC_DMA_RBU_FLAG))
  {
    /* Clear RBUS ETHERNET DMA flag */
    emac_dma_flag_clear(EMAC_DMA_RBU_FLAG);
    /* Resume DMA reception */
    EMAC_DMA->rpd_bit.rpd = FALSE;
  }


  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t
ethernetif_input_E(struct netif *netif)
{
  err_t err;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

  err = netif->input(p, netif);
  if (err != ERR_OK)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input_E: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }

  return err;
}

/*******************************************************************************
* Function Name  : emac_rxpkt_chainmode
* Description    : Receives a packet.
* Input          : None
* Output         : None
* Return         : frame: farme size and location
*******************************************************************************/
FrameTypeDef emac_rxpkt_chainmode(void)
{
  u32 framelength = 0;
  FrameTypeDef frame = {0,0};

  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((dma_rx_desc_to_get->status & EMAC_DMARXDESC_OWN) != (u32)RESET)
  {
    frame.length = FALSE;

    if(emac_dma_flag_get(EMAC_DMA_RBU_FLAG))
    {
      /* Clear RBUS ETHERNET DMA flag */
      emac_dma_flag_clear(EMAC_DMA_RBU_FLAG);
      /* Resume DMA reception */
      EMAC_DMA->rpd_bit.rpd = FALSE;
    }
    /* Return error: OWN bit set */
    return frame;
  }

  if(((dma_rx_desc_to_get->status & EMAC_DMATXDESC_ES) == (u32)RESET) &&
     ((dma_rx_desc_to_get->status & EMAC_DMARXDESC_LS) != (u32)RESET) &&
     ((dma_rx_desc_to_get->status & EMAC_DMARXDESC_FS) != (u32)RESET))
  {
    /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
    framelength = ((dma_rx_desc_to_get->status & EMAC_DMARXDESC_FL) >> EMAC_DMARxDesc_FrameLengthShift) - 4;

    /* Get the addrees of the actual buffer */
    frame.buffer = dma_rx_desc_to_get->buf1addr;
  }
  else
  {
    /* Return ERROR */
    framelength = FALSE;
  }

  frame.length = framelength;

  frame.descriptor = dma_rx_desc_to_get;

  /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */
  /* Chained Mode */
  /* Selects the next DMA Rx descriptor list for next buffer to read */
  dma_rx_desc_to_get = (emac_dma_desc_type*) (dma_rx_desc_to_get->buf2nextdescaddr);
  /* Return Frame */
  return (frame);
}

/*******************************************************************************
* Function Name  : emac_txpkt_chainmode
* Description    : Transmits a packet, from application buffer, pointed by ppkt.
* Input          : - FrameLength: Tx Packet size.
* Output         : None
* Return         : ERROR: in case of Tx desc owned by DMA
*                  SUCCESS: for correct transmission
*******************************************************************************/
error_status emac_txpkt_chainmode(u16 FrameLength)
{
  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((dma_tx_desc_to_set->status & EMAC_DMATXDESC_OWN) != (u32)RESET)
  {
    /* Return ERROR: OWN bit set */
    return ERROR;
  }

  /* Setting the Frame Length: bits[12:0] */
  dma_tx_desc_to_set->controlsize = (FrameLength & EMAC_DMATXDESC_TBS1);

  /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
  dma_tx_desc_to_set->status |= EMAC_DMATXDESC_LS | EMAC_DMATXDESC_FS;

  /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
  dma_tx_desc_to_set->status |= EMAC_DMATXDESC_OWN;
  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if(emac_dma_flag_get(EMAC_DMA_TBU_FLAG))
  {
    /* Clear TBUS ETHERNET DMA flag */
    emac_dma_flag_clear(EMAC_DMA_TBU_FLAG);
    /* Resume DMA transmission*/
    EMAC_DMA->tpd_bit.tpd = 0;
  }

  /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */
  /* Chained Mode */
  /* Selects the next DMA Tx descriptor list for next buffer to send */
  dma_tx_desc_to_set = (emac_dma_desc_type*) (dma_tx_desc_to_set->buf2nextdescaddr);
  /* Return SUCCESS */
  return SUCCESS;
}


/*******************************************************************************
* Function Name  : emac_getcurrenttxbuffer
* Description    : Return the address of the buffer pointed by the current descritor.
* Input          : None
* Output         : None
* Return         : Buffer address
*******************************************************************************/
u32 emac_getcurrenttxbuffer(void)
{
  /* Return Buffer address */
  return (dma_tx_desc_to_set->buf1addr);
}

u32_t emac_ptpsubsecond2nanosecond(u32_t subsecondvalue)
{
  uint64_t val = subsecondvalue * 1000000000ll;
  val >>= 31;
  return (u32_t)val;
}

u32_t emac_ptpnanosecond2subsecond(u32_t subsecondvalue)
{
  uint64_t val = subsecondvalue * 0x80000000ll;
  val /= 1000000000;
  return val;
}

void emac_ptptime_gettime(struct ptptime_t * timestamp) {
  timestamp->tv_nsec = emac_ptpsubsecond2nanosecond(EMAC_PTP->tsl);
  timestamp->tv_sec = EMAC_PTP->tsh;
}

void emac_ptptime_settime(struct ptptime_t * timestamp)
{
    uint32_t Sign;
    uint32_t SecondValue;
    uint32_t NanoSecondValue;
    uint32_t SubSecondValue;

    /* determine sign and correct Second and Nanosecond values */
    if(timestamp->tv_sec < 0 || (timestamp->tv_sec == 0 && timestamp->tv_nsec < 0)) {
        Sign = EMAC_PTP_NEGATIVETIME;
        SecondValue = -timestamp->tv_sec;
        NanoSecondValue = -timestamp->tv_nsec;
    }
    else {
        Sign = EMAC_PTP_POSITIVETIME;
        SecondValue = timestamp->tv_sec;
        NanoSecondValue = timestamp->tv_nsec;
    }

    /* convert nanosecond to subseconds */
    SubSecondValue = emac_ptpnanosecond2subsecond(NanoSecondValue);
 
    /* Write the offset (positive or negative) in the Time stamp update high and low registers. */
    emac_ptp_system_time_set(Sign, SecondValue, SubSecondValue);
    /* Set Time stamp control register bit 2 (Time stamp init). */
    emac_ptp_timestamp_system_time_init(TRUE);
    /* The Time stamp counter starts operation as soon as it is initialized
     * with the value written in the Time stamp update register. */
    while(emac_ptp_flag_get(EMAC_PTP_TI_FLAG));
}

void emac_ptptime_adjfreq(int32_t adj)
{
    uint32_t addend;

    
    /* calculate the rate by which you want to speed up or slow down the system time
       increments */
   
    /* precise */
    /*
    int64_t addend;
    addend = Adj;
    addend *= ADJ_FREQ_BASE_ADDEND;
    addend /= 1000000000-Adj;
    addend += ADJ_FREQ_BASE_ADDEND;
    */

    /* 32bit estimation
    ADJ_LIMIT = ((1l<<63)/275/ADJ_FREQ_BASE_ADDEND) = 11258181 = 11 258 ppm*/
    if( adj > 5120000) adj = 5120000;
    if( adj < -5120000) adj = -5120000;

    addend = ((((275LL * adj)>>8) * (ADJ_FREQ_BASE_ADDEND>>24))>>6) + ADJ_FREQ_BASE_ADDEND;
    
    /* Reprogram the Time stamp addend register with new Rate value and set ETH_TPTSCR */
    emac_ptp_timestamp_addend_set(addend);
    emac_ptp_addend_register_update(TRUE);
}

#if 0 //peek~
void emac_ptptime_updateoffset(struct ptptime_t * timeoffset)
{
    uint32_t Sign;
    uint32_t SecondValue;
    uint32_t NanoSecondValue;
    uint32_t SubSecondValue;
    uint32_t addend;

    /* determine sign and correct Second and Nanosecond values */
    if(timeoffset->tv_sec < 0 || (timeoffset->tv_sec == 0 && timeoffset->tv_nsec < 0)) {
        Sign = EMAC_PTP_NEGATIVETIME;
        SecondValue = -timeoffset->tv_sec;
        NanoSecondValue = -timeoffset->tv_nsec;
    } else {
        Sign = EMAC_PTP_POSITIVETIME;
        SecondValue = timeoffset->tv_sec;
        NanoSecondValue = timeoffset->tv_nsec;
    }

    /* convert nanosecond to subseconds */
    SubSecondValue = emac_ptpnanosecond2subsecond(NanoSecondValue);
 
    /* read old addend register value*/
    addend = EMAC_PTP->tsad;

    while(emac_ptp_flag_get(EMAC_PTP_TU_FLAG));
    while(emac_ptp_flag_get(EMAC_PTP_TI_FLAG));

    /* Write the offset (positive or negative) in the Time stamp update high and low registers. */
    emac_ptp_system_time_set(Sign, SecondValue, SubSecondValue);
    /* Set bit 3 (TSSTU) in the Time stamp control register. */
    emac_ptp_timestamp_system_time_update(TRUE);
    /* The value in the Time stamp update registers is added to or subtracted from the system */
    /* time when the TSSTU bit is cleared. */
    while(emac_ptp_flag_get(EMAC_PTP_TU_FLAG));

    /* write back old addend register value */
    emac_ptp_timestamp_addend_set(addend);
    emac_ptp_addend_register_update(TRUE);
}
#endif
#endif
