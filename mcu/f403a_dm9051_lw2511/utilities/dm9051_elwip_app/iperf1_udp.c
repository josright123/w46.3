#include "iperf1.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/udp.h"

#include <string.h>

#if 1
/**
  * @brief  initialize iperf server
  * @param  none
  * @retval none
  */
//void iperf3_udp_init(void);
//void iperf3_udp_init(void)
//{

//    // Initialize UDP server
//    struct udp_pcb *udp_pcb;
//    iperf3_state_t *udp_state = (iperf3_state_t *)LWIPERF_ALLOC(iperf3_state_t);

//    if (udp_state == NULL) {
//        printf("[iPerf3] UDP server initialization failed - memory allocation error\r\n");
//        return;
//    }
//    
//    memset(udp_state, 0, sizeof(iperf3_state_t));
//    udp_state->is_server = 1;
//    udp_state->protocol = 1; // UDP
//    udp_state->report_arg = NULL;
//    udp_state->state = IPERF3_TEST_START;
//    printf("[iPerf3] init: creating UDP server on %u\r\n", IPERF3_DEFAULT_PORT);

//    udp_pcb = udp_new();
//    if (udp_pcb == NULL) {
//        LWIPERF_FREE(iperf3_state_t, udp_state);
//        printf("[iPerf3] UDP PCB creation failed\r\n");
//        return;
//    }

//    if (udp_bind(udp_pcb, IP_ADDR_ANY, IPERF3_DEFAULT_PORT) != ERR_OK) {
//        udp_remove(udp_pcb);
//        LWIPERF_FREE(iperf3_state_t, udp_state);
//        printf("[iPerf3] UDP bind failed\r\n");
//        return;
//    }

//    udp_state->pcb.udp = udp_pcb;
//    udp_state->time_started_udp = sys_now();
//    udp_recv(udp_pcb, iperf3_udp_recv, udp_state);
//    printf("[iPerf3] UDP server listening on port %d\r\n", IPERF3_DEFAULT_PORT);
//    
////    printf("[iPerf3] Server initialized successfully\r\n");
//}

/** Iperf2 */
typedef struct _lwiperf_state_udp {
  struct udp_pcb *udp_pcb;
  u32_t time_started_udp;
  void *report_arg;
  u32_t bytes_transferred;
  u32_t packet_count;
  u32_t last_bytes;        // Last interval bytes count
  u32_t last_time;         // Last interval time
  u32_t interval_packets;  // Packet count in current interval
  ip_addr_t remote_addr;
  u16_t remote_port;
} lwiperf_state_udp_t;

/* Iperf UDP packet header */
typedef struct {
    uint32_t id;
    uint32_t tv_sec;
    uint32_t tv_usec;
} iperf_udp_header_t;

//** [udp] */
static u32_t leave_on_last_now = 0;
static u32_t acc_interval_show = 0;

static void iperf_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                          const ip_addr_t *addr, u16_t port) {
  lwiperf_state_udp_t *conn = (lwiperf_state_udp_t *)arg;

  if (p != NULL) {
    if (p->len >= sizeof(iperf_udp_header_t)) {
      // Process received data
      conn->bytes_transferred += p->tot_len;
      conn->packet_count++;
      conn->interval_packets++;
      conn->remote_addr = *addr;
      conn->remote_port = port;

      // Send ACK back to client every 10 packets to reduce overhead
      if (conn->packet_count % 10 == 0) {
        static struct pbuf *ack_pbuf = NULL;
        if (ack_pbuf == NULL) {
            ack_pbuf = pbuf_alloc(PBUF_TRANSPORT, sizeof(iperf_udp_header_t), PBUF_RAM);
        }
        if (ack_pbuf != NULL) {
            iperf_udp_header_t *hdr = (iperf_udp_header_t *)p->payload;
            iperf_udp_header_t *ack_hdr = (iperf_udp_header_t *)ack_pbuf->payload;

            // Copy the packet ID and timestamp from received packet
            ack_hdr->id = hdr->id;
            ack_hdr->tv_sec = hdr->tv_sec;
            ack_hdr->tv_usec = hdr->tv_usec;

            // Send ACK back to client
            udp_sendto(pcb, ack_pbuf, addr, port);
        }
      }

      // Print progress every 1000 packets
	  u32_t increase_interval = 1500;
      if (conn->interval_packets >= (acc_interval_show + increase_interval)) {

		//[acc_interval_show]
		if (!acc_interval_show)
			printf("\r\n[IPERF2 UDP] -----[s] acc_interval_show test start (Interval %lu) -----\r\n", acc_interval_show);
		  
        u32_t now = sys_now();
        u32_t bytes_delta = conn->bytes_transferred - conn->last_bytes;
        u32_t time_delta = now - (conn->last_time ? conn->last_time : conn->time_started_udp);

        if (time_delta > 0) {
            u32_t bandwidth_kbitpsec = (bytes_delta * 8) / time_delta;
            printf("[IPERF2 UDP] %u packets, %u bytes, %u.%03u Mbits/sec (Interval)\r\n",
                   conn->interval_packets,
                   bytes_delta,
                   bandwidth_kbitpsec / 1000,
                   bandwidth_kbitpsec % 1000);
        }

        // Update last values for next interval
        conn->last_bytes = conn->bytes_transferred;
        conn->last_time = now;
        conn->interval_packets = 0; // Reset interval packet counter
		acc_interval_show += increase_interval;
      }
    }
    pbuf_free(p);

	leave_on_last_now = sys_now();
  }
}
						  
void iperf2_udp_last_time_manager(void)
{
  #define DEF_LAST_NOW_FAR	6000
  if (acc_interval_show && ((sys_now() - leave_on_last_now) > DEF_LAST_NOW_FAR))
  {
	printf("[IPERF2 UDP] -----[e] acc_interval_show (Interval reset clear %lu to %d) -----\r\n", acc_interval_show, 0);
	acc_interval_show = 0;
  }
}

void iperf2_udp_init(void)
{
    // Initialize UDP server
    struct udp_pcb *udp_pcb;
    lwiperf_state_udp_t *s_udp = (lwiperf_state_udp_t *)LWIPERF_ALLOC(lwiperf_state_udp_t);

    if (s_udp == NULL) {
        printf("iperf_init UDP err_mem\r\n");
        return;
    }
    s_udp->time_started_udp = 0;
    s_udp->bytes_transferred = 0;
    s_udp->packet_count = 0;
    s_udp->last_bytes = 0;
    s_udp->last_time = 0;
    s_udp->interval_packets = 0;
    s_udp->report_arg = NULL;

    udp_pcb = udp_new();
    if (udp_pcb == NULL) {
        LWIPERF_FREE(lwiperf_state_udp_t, s_udp);
        printf("iperf_init UDP PCB creation failed\r\n");
        return;
    }

    if (udp_bind(udp_pcb, IP_ADDR_ANY, IPERF2_DEFAULT_PORT) != ERR_OK) { // bind to iperf UDP port 5001
        udp_remove(udp_pcb);
        LWIPERF_FREE(lwiperf_state_udp_t, s_udp);
        printf("iperf_init UDP bind failed\r\n");
        return;
    }

    s_udp->udp_pcb = udp_pcb;
    s_udp->time_started_udp = sys_now();
    udp_recv(udp_pcb, iperf_udp_recv, s_udp);
    printf("iPerf2 UDP server listening on port %d\r\n", IPERF2_DEFAULT_PORT);
}
#endif
