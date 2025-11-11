/**
  **************************************************************************
  * @file     iperf3.c
  * @brief    iPerf3 compatible network performance measurement tool
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
  * Upgraded to iPerf3 protocol support, 20250918
  */

#include "iperf.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/udp.h"

#include <string.h>

/* iPerf3 protocol constants */

//#define IPERF2_DEFAULT_PORT 5001
//#define IPERF3_DEFAULT_PORT 5201

	#define IPERF2_DEFAULT_PORT 5000
	#define IPERF3_DEFAULT_PORT 5001

#define IPERF3_COOKIE_SIZE 37
#define IPERF3_DEFAULT_UDP_RATE (1024 * 1024) /* 1 Mbps */

/* iPerf3 test states */
typedef enum {
    IPERF3_TEST_START = 1,
    IPERF3_TEST_RUNNING,
    IPERF3_TEST_END
} iperf3_run_state_t;

/* iPerf3 protocol message types */
typedef enum {
    IPERF3_MSG_COOKIE = 1,
    IPERF3_MSG_TEST_START,
    IPERF3_MSG_TEST_RUNNING,
    IPERF3_MSG_TEST_END,
    IPERF3_MSG_PARAM_EXCHANGE
} iperf3_msg_type_t;

/* iPerf3 test parameters */
typedef struct {
    uint32_t flags;           /* Test flags */
    uint32_t num_streams;     /* Number of parallel streams */
    uint32_t blksize;        /* Block size for data transfer */
    uint32_t rate;           /* Target data rate for UDP */
    uint32_t duration;       /* Test duration in seconds */
    uint8_t  cookie[IPERF3_COOKIE_SIZE]; /* Test cookie */
} iperf3_test_params_t;

/* iPerf3 UDP packet header */
typedef struct {
    uint32_t id;             /* Packet ID */
    uint32_t tv_sec;         /* Timestamp seconds */
    uint32_t tv_usec;        /* Timestamp microseconds */
    iperf3_msg_type_t type;  /* Message type */
    uint32_t remaining;      /* Remaining packets in burst */
    uint32_t flags;          /* Packet flags */
} iperf3_udp_header_t;

/* File internal memory allocation (struct lwiperf_*): this defaults to
   the heap */
#ifndef LWIPERF_ALLOC
#define LWIPERF_ALLOC(type)         mem_malloc(sizeof(type))
#define LWIPERF_FREE(type, item)    mem_free(item)
#endif

/** iPerf3 session state */
typedef struct _iperf3_state {
    union {
        struct tcp_pcb *tcp;
        struct udp_pcb *udp;
    } pcb;
    iperf3_run_state_t state;
    iperf3_test_params_t params;
    void *report_arg;
    u32_t time_started;
    u32_t time_last_report;
    u32_t bytes_transferred;
    u32_t bytes_last_report;
    u32_t packet_count;
    u32_t packet_errors;
    u32_t jitter_us;        /* Jitter in microseconds */
    u32_t packet_loss;      /* Lost packets count */
    ip_addr_t remote_addr;
    u16_t remote_port;
    u8_t is_server;
    u8_t protocol;         /* 0 for TCP, 1 for UDP */
    u8_t reverse_mode;     /* 1 if server sends data */
    struct _iperf3_state *next; /* For multiple parallel streams */
} iperf3_state_t;

/**
 * @brief  lwIP iPerf report callback function
 * @note   Called when iPerf test is completed, used to display test results
 * @param  arg: User defined parameter
 * @param  report_type: Test result type
 * @param  local_addr: Local IP address
 * @param  local_port: Local port number
 * @param  remote_addr: Remote IP address
 * @param  remote_port: Remote port number
 * @param  bytes_transferred: Number of bytes transferred
 * @param  ms_duration: Test duration (milliseconds)
 * @param  bandwidth_kbitpsec: Bandwidth (Kbps)
 * @retval None
 * (main.c)
 */

/** This is the Iperf settings struct sent from the client */
typedef struct _lwiperf_settings {
#define LWIPERF_FLAGS_ANSWER_TEST 0x80000000
#define LWIPERF_FLAGS_ANSWER_NOW  0x00000001
  u32_t flags;
  u32_t num_threads; /* unused for now */
  u32_t remote_port;
  u32_t buffer_len; /* unused for now */
  u32_t win_band; /* TCP window / UDP rate: unused for now */
  u32_t amount; /* pos. value: bytes?; neg. values: time (unit is 10ms: 1/100 second) */
} lwiperf_settings_t;

typedef struct _lwiperf_state_base lwiperf_state_base_t;
struct _lwiperf_state_base {
  /* linked list */
  lwiperf_state_base_t *next;
  /* 1=tcp, 0=udp */
  u8_t tcp;
  /* 1=server, 0=client */
  u8_t server;
  /* master state used to abort sessions (e.g. listener, main client) */
  lwiperf_state_base_t *related_master_state;
};

/** Connection handle for a TCP iperf session */
typedef struct _lwiperf_state_tcp {
  lwiperf_state_base_t base;
  struct tcp_pcb *server_pcb;
  struct tcp_pcb *conn_pcb;
  u32_t time_started;
  lwiperf_report_fn report_fn;
  void *report_arg;
  u8_t poll_count;
  u8_t next_num;
  /* 1=start server when client is closed */
  u8_t client_tradeoff_mode;
  u32_t bytes_transferred;
  lwiperf_settings_t settings;
  u8_t have_settings_buf;
  u8_t specific_remote;
  ip_addr_t remote_addr;
} lwiperf_state_tcp_t;

static void iperf_report_callback(void *arg, enum lwiperf_report_type report_type, 
                                   const ip_addr_t *local_addr, u16_t local_port,
                                   const ip_addr_t *remote_addr, u16_t remote_port,
                                   u32_t bytes_transferred, u32_t ms_duration, 
                                   u32_t bandwidth_kbitpsec /*, iperf3_state_t *state*/)
{
	//iperf3_report_callback();
}

#if 0
static void iperf3_report_callback(void *arg, enum lwiperf_report_type report_type, 
                                   const ip_addr_t *local_addr, u16_t local_port,
                                   const ip_addr_t *remote_addr, u16_t remote_port,
                                   u32_t bytes_transferred, u32_t ms_duration, 
                                   u32_t bandwidth_kbitpsec, iperf3_state_t *state)
{
    const char *test_type = state ? (state->protocol == 0 ? "TCP" : "UDP") : "Unknown";
    
    switch (report_type)
    {
        case LWIPERF_TCP_DONE_SERVER:
            printf("\r\n[iPerf3] %s Server test completed\r\n", test_type);
            break;
        case LWIPERF_TCP_DONE_CLIENT:
            printf("[iPerf3] %s Client test completed\r\n", test_type);
            break;
        case LWIPERF_TCP_ABORTED_LOCAL:
            printf("[iPerf3] %s Local error aborted\r\n", test_type);
            break;
        case LWIPERF_TCP_ABORTED_LOCAL_DATAERROR:
            printf("[iPerf3] %s Local data error aborted\r\n", test_type);
            break;
        case LWIPERF_TCP_ABORTED_LOCAL_TXERROR:
            printf("[iPerf3] %s Local transmission error aborted\r\n", test_type);
            break;
        case LWIPERF_TCP_ABORTED_REMOTE:
            printf("[iPerf3] %s Remote error aborted\r\n", test_type);
            break;
        default:
            printf("[iPerf3] %s Other error\r\n", test_type);
            break;
    }

    if (bandwidth_kbitpsec > 0)
    {
        /* Use integer arithmetic to avoid float formatting issues */
        u32_t mbits_int = bandwidth_kbitpsec / 1024;
        u32_t mbits_frac = (bandwidth_kbitpsec % 1024) * 100 / 1024;
        u32_t mbytes_int = bytes_transferred / (1024 * 1024);
        u32_t mbytes_frac = (bytes_transferred % (1024 * 1024)) * 100 / (1024 * 1024);
        u32_t sec_duration = ms_duration / 1000;
        u32_t sec_frac = (ms_duration % 1000) / 100;

        // Connection info
        printf("[iPerf3] Local %d.%d.%d.%d port %u connected with %d.%d.%d.%d port %u\r\n",
            ip4_addr1_val(*local_addr), ip4_addr2_val(*local_addr), 
            ip4_addr3_val(*local_addr), ip4_addr4_val(*local_addr), local_port,
            ip4_addr1_val(*remote_addr), ip4_addr2_val(*remote_addr), 
            ip4_addr3_val(*remote_addr), ip4_addr4_val(*remote_addr), remote_port);

        // Basic metrics
        printf("[iPerf3] Interval: 0.0- %u.%01u sec\r\n", sec_duration, sec_frac);
        printf("[iPerf3] Transfer: %u.%02u MBytes\r\n", mbytes_int, mbytes_frac);
        printf("[iPerf3] Bandwidth: %u.%02u Mbits/sec\r\n", mbits_int, mbits_frac);

        // Additional iPerf3 specific metrics for UDP
        if (state && state->protocol == 1) { // UDP
            u32_t packet_loss_percent = 0;
            if (state->packet_count > 0) {
                packet_loss_percent = (state->packet_loss * 100) / state->packet_count;
            }
            
            printf("[iPerf3] UDP Stats:\r\n");
            printf("         Total Datagrams: %u\r\n", state->packet_count);
            printf("         Lost Datagrams: %u (%u%%)\r\n", 
                   state->packet_loss, packet_loss_percent);
            printf("         Jitter: %u.%03u ms\r\n", 
                   state->jitter_us / 1000, state->jitter_us % 1000);
        }

        // Additional iPerf3 specific metrics for TCP
        if (state && state->protocol == 0) { // TCP
            printf("[iPerf3] TCP Stats:\r\n");
#if 0
            printf("         Retr: %u\r\n", tcp_sndbuf); // Example TCP metric
#endif
        }
    }
    else
    {
        printf("[iPerf3] No bandwidth data available\r\n");
    }
}
#endif

static void iperf3_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                           const ip_addr_t *addr, u16_t port) {
    iperf3_state_t *state = (iperf3_state_t *)arg;
	u32_t now;
	u32_t test_duration;
	u32_t transit_time;
    
    if (p == NULL) {
        printf("[iPerf3][UDP] recv: p==NULL from %d.%d.%d.%d:%u\r\n",
               ip4_addr1_val(*addr), ip4_addr2_val(*addr), ip4_addr3_val(*addr), ip4_addr4_val(*addr), port);
        return;
    }

    if (p->len < sizeof(iperf3_udp_header_t)) {
        printf("[iPerf3][UDP] recv: short packet len=%u from %d.%d.%d.%d:%u\r\n",
               (unsigned)p->len,
               ip4_addr1_val(*addr), ip4_addr2_val(*addr), ip4_addr3_val(*addr), ip4_addr4_val(*addr), port);
        pbuf_free(p);
        return;
    }

    if (p != NULL && p->len >= sizeof(iperf3_udp_header_t)) {
        iperf3_udp_header_t *hdr = (iperf3_udp_header_t *)p->payload;
        printf("[iPerf3][UDP] recv: type=%u id=%u len=%u state=%u\r\n",
               (unsigned)hdr->type, (unsigned)hdr->id, (unsigned)p->tot_len, (unsigned)state->state);
        
        switch (hdr->type) {
            case IPERF3_MSG_TEST_START:
                // Initialize test parameters
                state->state = IPERF3_TEST_RUNNING;
                state->time_started = sys_now();
                state->time_last_report = state->time_started;
                state->remote_addr = *addr;
                state->remote_port = port;
                printf("[iPerf3][UDP] TEST_START: peer %d.%d.%d.%d:%u\r\n",
                       ip4_addr1_val(*addr), ip4_addr2_val(*addr), ip4_addr3_val(*addr), ip4_addr4_val(*addr), port);
                break;
                
            case IPERF3_MSG_TEST_RUNNING:
                // Process data packet
                state->bytes_transferred += p->tot_len;
                state->packet_count++;
                
                // Calculate jitter
                transit_time = (sys_now() - hdr->tv_sec) * 1000 + hdr->tv_usec;
                if (state->packet_count > 1) {
                    int32_t jitter_diff = transit_time - state->jitter_us;
                    state->jitter_us += jitter_diff / 16; // Exponential average
                }
                
                // Send statistics every 100 packets
                if (state->packet_count % 100 == 0) {
                    struct pbuf *ack = pbuf_alloc(PBUF_TRANSPORT, 
                                                sizeof(iperf3_udp_header_t), 
                                                PBUF_RAM);
                    if (ack != NULL) {
                        iperf3_udp_header_t *ack_hdr = (iperf3_udp_header_t *)ack->payload;
                        ack_hdr->id = hdr->id;
                        ack_hdr->type = IPERF3_MSG_TEST_RUNNING;
                        ack_hdr->flags = 0;
                        ack_hdr->remaining = 0;
                        
                        now = sys_now();
                        ack_hdr->tv_sec = now / 1000;
                        ack_hdr->tv_usec = (now % 1000) * 1000;
                        
                        udp_sendto(pcb, ack, addr, port);
                        pbuf_free(ack);
                        
                        // Generate interim report
                        if ((now - state->time_last_report) >= 1000) {
                            u32_t bytes_since_report = state->bytes_transferred - 
                                                     state->bytes_last_report;
                            printf("[iPerf3][UDP] interim: pkts=%u bytes=%u bw=%u kbit/s\r\n",
                                   (unsigned)state->packet_count,
                                   (unsigned)bytes_since_report,
                                   (unsigned)((bytes_since_report * 8) / (now - state->time_last_report)));
                            iperf_report_callback(state->report_arg,
                                               LWIPERF_TCP_DONE_SERVER,
                                               &pcb->local_ip, pcb->local_port,
                                               addr, port,
                                               bytes_since_report,
                                               now - state->time_last_report,
                                               (bytes_since_report * 8) / 
                                               (now - state->time_last_report));
                            
                            state->time_last_report = now;
                            state->bytes_last_report = state->bytes_transferred;
                        }
                    }
                }
                break;
                
            case IPERF3_MSG_TEST_END:
                // Generate final report
                now = sys_now();
                test_duration = now - state->time_started;
                printf("[iPerf3][UDP] TEST_END: duration=%u ms bytes=%u\r\n",
                       (unsigned)test_duration, (unsigned)state->bytes_transferred);
                
                iperf_report_callback(state->report_arg,
                                   LWIPERF_TCP_DONE_SERVER,
                                   &pcb->local_ip, pcb->local_port,
                                   addr, port,
                                   state->bytes_transferred,
                                   test_duration,
                                   (state->bytes_transferred * 8) / test_duration);
                
                // Reset state for next test
                state->bytes_transferred = 0;
                state->packet_count = 0;
                state->jitter_us = 0;
                state->packet_loss = 0;
                state->state = IPERF3_TEST_START;
                break;
                
            default:
                // Ignore unknown message types
                printf("[iPerf3][UDP] recv: unknown type=%u\r\n", (unsigned)hdr->type);
                break;
        }
        
        pbuf_free(p);
    }
}

/** Call the report function of an iperf tcp session */
#if 0
static void
lwip_tcp_conn_report(lwiperf_state_tcp_t *conn, enum lwiperf_report_type report_type)
{
  if ((conn != NULL) /*&& (conn->report_fn != NULL)*/) {
    u32_t now, duration_ms, bandwidth_kbitpsec;
    now = sys_now();
    duration_ms = now - conn->time_started;
	if (duration_ms == 0) {
      bandwidth_kbitpsec = 0;
	} else {
      /* Calculate bandwidth in kbits/sec with overflow protection */
      /* Use 64-bit arithmetic to prevent overflow */
      u64_t bytes_transferred_64 = (u64_t)conn->bytes_transferred;
      u64_t duration_ms_64 = (u64_t)duration_ms;
      u64_t bytes_per_ms_64 = bytes_transferred_64 / duration_ms_64;

      /* Check for overflow in final calculation */
      bandwidth_kbitpsec = (u32_t)(bytes_per_ms_64 * 8ULL);
	}


//    conn->report_fn(conn->report_arg, report_type,
//                    &conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
//                    &conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port,
//                    conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
    iperf_report_callback(conn->report_arg, report_type,
                    &conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
                    &conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port,
                    conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
  }
}
#endif

/**
  * @brief  iperf receive callback
  * @param  arg: the user argument
  * @param  pcb: the tcp_pcb that has received the data
  * @param  p: the packet buffer
  * @param  err: the error value linked with the received data
  * @retval error value
  */
static void iperf3_handle_client_message(iperf3_state_t *state, char *data, u16_t len);

static err_t iperf3_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    iperf3_state_t *state = (iperf3_state_t *)arg;
    
    if (err == ERR_OK && p != NULL) {
        printf("[iPerf3][TCP] recv: len=%u state=%u from %d.%d.%d.%d:%u\r\n",
               (unsigned)p->tot_len, (unsigned)state->state,
               ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
               ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
               pcb->remote_port);
        if (state->state == IPERF3_TEST_START) {
            // Handle initial protocol negotiation
            iperf3_handle_client_message(state, (char *)p->payload, p->len);
        } else {
            // Normal data transfer
            state->bytes_transferred += p->tot_len;
            
            // Calculate time elapsed since last report
            u32_t now = sys_now();
            if ((now - state->time_last_report) >= 1000) { // Report every second
                u32_t bytes_since_report = state->bytes_transferred - state->bytes_last_report;
                printf("[iPerf3][TCP] interim: bytes=%u bw=%u kbit/s\r\n",
                       (unsigned)bytes_since_report,
                       (unsigned)((bytes_since_report * 8) / (now - state->time_last_report)));
                // Generate interim report
                iperf_report_callback(state->report_arg, 
                                    LWIPERF_TCP_DONE_SERVER,
                                    &pcb->local_ip, pcb->local_port,
                                    &pcb->remote_ip, pcb->remote_port,
                                    bytes_since_report, 
                                    now - state->time_last_report,
                                    (bytes_since_report * 8) / (now - state->time_last_report));
                
                state->time_last_report = now;
                state->bytes_last_report = state->bytes_transferred;
            }
        }
        tcp_recved(pcb, p->tot_len);
        pbuf_free(p);
    } else if (err != ERR_OK && p != NULL) {
        printf("[iPerf3][TCP] recv error: err=%d, dropping p len=%u\r\n", err, (unsigned)p->tot_len);
        pbuf_free(p);
    }

    if (err == ERR_OK && p == NULL) {
        // Connection closed by peer
        printf("[iPerf3][TCP] connection closed by peer %d.%d.%d.%d:%u state=%u\r\n",
               ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
               ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
               pcb->remote_port, (unsigned)state->state);
        if (state->state == IPERF3_TEST_RUNNING) {
            // Generate final report
            u32_t now = sys_now();
            iperf_report_callback(state->report_arg,
                                LWIPERF_TCP_DONE_SERVER,
                                &pcb->local_ip, pcb->local_port,
                                &pcb->remote_ip, pcb->remote_port,
                                state->bytes_transferred,
                                now - state->time_started,
                                (state->bytes_transferred * 8) / (now - state->time_started));
        }
        
        // Clean up connection
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_close(pcb);
        
        // Free state
        LWIPERF_FREE(iperf3_state_t, state);
    }
    return ERR_OK;
}

/**
  * @brief  accept iperf connection
  * @param  arg: user supplied argument
  * @param  pcb: the tcp_pcb which accepted the connection
  * @param  err: error value
  * @retval error value
  */
static void iperf3_handle_client_message(iperf3_state_t *state, char *data, u16_t len)
{
    printf("[iPerf3][TCP] handle_client_message: len=%u\r\n", (unsigned)len);

    /* iperf3 v3.x first sends a 37-byte ASCII cookie on the control TCP
       connection. Accept and echo it to proceed with the test. */
    if (len == IPERF3_COOKIE_SIZE) {
        memcpy(state->params.cookie, data, IPERF3_COOKIE_SIZE);
        err_t werr = tcp_write(state->pcb.tcp, state->params.cookie, IPERF3_COOKIE_SIZE, TCP_WRITE_FLAG_COPY);
        if (werr != ERR_OK) {
            printf("[iPerf3][TCP] cookie echo write failed: %d\r\n", werr);
        } else {
            werr = tcp_output(state->pcb.tcp);
            if (werr != ERR_OK) {
                printf("[iPerf3][TCP] cookie tcp_output failed: %d\r\n", werr);
            }
        }
        state->state = IPERF3_TEST_RUNNING;
        state->time_started = sys_now();
        state->time_last_report = state->time_started;
        printf("[iPerf3][TCP] cookie accepted, state -> RUNNING\r\n");
        return;
    }

    /* Minimal control op handling: iperf3 v3.x may send 1-byte control
       messages (e.g., PARAM_EXCHANGE/TEST_START/etc.). Echo them so the
       client proceeds. */
    if (len == 1) {
        unsigned char opcode = (unsigned char)data[0];
        printf("[iPerf3][TCP] ctrl opcode=0x%02x\r\n", (unsigned)opcode);
        /* Do not echo unknown 1-byte control to avoid confusing client. */
        if (opcode == 0x01) {
            state->state = IPERF3_TEST_RUNNING;
            state->time_started = sys_now();
            state->time_last_report = state->time_started;
            printf("[iPerf3][TCP] ctrl start, state -> RUNNING\r\n");
        } else if (opcode == 0x02) {
            state->state = IPERF3_TEST_END;
            printf("[iPerf3][TCP] ctrl end, state -> END\r\n");
        }
        return;
    }

    if (len < sizeof(iperf3_test_params_t)) {
        printf("[iPerf3][TCP] params too short: need %u got %u\r\n",
               (unsigned)sizeof(iperf3_test_params_t), (unsigned)len);
        return; // Invalid message
    }

    iperf3_test_params_t *params = (iperf3_test_params_t *)data;
    
    // Store test parameters
    memcpy(&state->params, params, sizeof(iperf3_test_params_t));
    printf("[iPerf3][TCP] params: flags=0x%08x streams=%u blksize=%u rate=%u dur=%u\r\n",
           (unsigned)state->params.flags,
           (unsigned)state->params.num_streams,
           (unsigned)state->params.blksize,
           (unsigned)state->params.rate,
           (unsigned)state->params.duration);
    
    // Validate and adjust parameters if needed
    if (state->params.blksize == 0) {
        state->params.blksize = 128 * 1024; // Default 128KB
        printf("[iPerf3][TCP] adjust blksize to %u\r\n", (unsigned)state->params.blksize);
    }
    if (state->params.duration == 0) {
        state->params.duration = 10; // Default 10 seconds
        printf("[iPerf3][TCP] adjust duration to %u\r\n", (unsigned)state->params.duration);
    }
    
    // Send back parameters acknowledgment
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(iperf3_test_params_t), PBUF_RAM);
    if (p != NULL) {
        memcpy(p->payload, &state->params, sizeof(iperf3_test_params_t));
        err_t werr = tcp_write(state->pcb.tcp, p->payload, p->len, TCP_WRITE_FLAG_COPY);
        if (werr != ERR_OK) {
            printf("[iPerf3][TCP] tcp_write ack failed: %d\r\n", werr);
        }
        werr = tcp_output(state->pcb.tcp);
        if (werr != ERR_OK) {
            printf("[iPerf3][TCP] tcp_output failed: %d\r\n", werr);
        }
        pbuf_free(p);
    } else {
        printf("[iPerf3][TCP] pbuf_alloc failed for params ack\r\n");
    }
    
    // Update state
    state->state = IPERF3_TEST_RUNNING;
    state->time_started = sys_now();
    state->time_last_report = state->time_started;
    printf("[iPerf3][TCP] state -> RUNNING\r\n");
}

static err_t iperf3_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    iperf3_state_t *server_state = (iperf3_state_t *)arg;
    iperf3_state_t *conn_state;
    LWIP_UNUSED_ARG(err);

    conn_state = (iperf3_state_t *)LWIPERF_ALLOC(iperf3_state_t);
    if (conn_state == NULL) {
        printf("[iPerf3][TCP] accept: ERR_MEM allocating state\r\n");
        return ERR_MEM;
    }
    
    memset(conn_state, 0, sizeof(iperf3_state_t));
    conn_state->pcb.tcp = pcb;
    conn_state->state = IPERF3_TEST_START;
    conn_state->time_started = sys_now();
    conn_state->report_arg = server_state->report_arg;
    conn_state->is_server = 1;
    conn_state->protocol = 0; // TCP
    printf("[iPerf3][TCP] accept: peer %d.%d.%d.%d:%u\r\n",
           ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
           ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
           pcb->remote_port);
    
    LWIPERF_FREE(iperf3_state_t, server_state);
    
    tcp_arg(pcb, conn_state);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, iperf3_recv);
    
    return ERR_OK;
}

/** Iperf2 */
typedef struct _lwiperf_state_udp {
  struct udp_pcb *udp_pcb;
  u32_t time_started;
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

//** [tcp] */
/** Call the report function of an iperf tcp session */
static void
lwip_tcp_conn_report(lwiperf_state_tcp_t *conn, enum lwiperf_report_type report_type)
{
  if ((conn != NULL) /*&& (conn->report_fn != NULL)*/) {
    u32_t now, duration_ms, bandwidth_kbitpsec;
    now = sys_now();
    duration_ms = now - conn->time_started;
	if (duration_ms == 0) {
      bandwidth_kbitpsec = 0;
	} else {
      /* Calculate bandwidth in kbits/sec with overflow protection */
      /* Use 64-bit arithmetic to prevent overflow */
      u64_t bytes_transferred_64 = (u64_t)conn->bytes_transferred;
      u64_t duration_ms_64 = (u64_t)duration_ms;
      u64_t bytes_per_ms_64 = bytes_transferred_64 / duration_ms_64;

      /* Check for overflow in final calculation */
      bandwidth_kbitpsec = (u32_t)(bytes_per_ms_64 * 8ULL);
	}


//    conn->report_fn(conn->report_arg, report_type,
//                    &conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
//                    &conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port,
//                    conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
    iperf_report_callback(conn->report_arg, report_type,
                    &conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
                    &conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port,
                    conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
  }
}

//** [udp] */
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
      if (conn->interval_packets >= 1000) {
        u32_t now = sys_now();
        u32_t bytes_delta = conn->bytes_transferred - conn->last_bytes;
        u32_t time_delta = now - (conn->last_time ? conn->last_time : conn->time_started);
        
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
      }
    }
    pbuf_free(p);
  }
}

/** Close an iperf tcp session */
static void
lwiperf_tcp_close(lwiperf_state_tcp_t *conn, enum lwiperf_report_type report_type)
{
	lwip_tcp_conn_report(conn, report_type);
  LWIPERF_FREE(lwiperf_state_tcp_t, conn);
}

/**
  * @brief  iperf receive callback
  * @param  arg: the user argument
  * @param  pcb: the tcp_pcb that has received the data
  * @param  p: the packet buffer
  * @param  err: the error value linked with the received data
  * @retval error value
  */
static err_t iperf2_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  lwiperf_state_tcp_t *conn = (lwiperf_state_tcp_t *)arg;

  if (err == ERR_OK && p != NULL) {

    conn->bytes_transferred += p->tot_len;
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
  } else
  if (err != ERR_OK && p != NULL) {
    pbuf_free(p);
  }

  if (err == ERR_OK && p == NULL) {
    lwiperf_tcp_close(conn, LWIPERF_TCP_DONE_SERVER);
    tcp_arg(pcb, NULL); //JJ.YES.NULL
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_close(pcb);
  }
  return ERR_OK;
}

/**
  * @brief  accept iperf connection
  * @param  arg: user supplied argument
  * @param  pcb: the tcp_pcb which accepted the connection
  * @param  err: error value
  * @retval error value
  */
static err_t iperf2_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  lwiperf_state_tcp_t *s, *conn;
  LWIP_UNUSED_ARG(err);

  s = (lwiperf_state_tcp_t *)arg;

  conn = (lwiperf_state_tcp_t *)LWIPERF_ALLOC(lwiperf_state_tcp_t);
  if (conn == NULL) {
    return ERR_MEM;
  }
  memset(conn, 0, sizeof(lwiperf_state_tcp_t));
  conn->conn_pcb = pcb;
  conn->time_started = sys_now();
  conn->report_arg = s->report_arg;
#if 1
  //[essential ]?
  LWIPERF_FREE(lwiperf_state_tcp_t, s);
#endif
  tcp_arg(pcb, conn); //JJ
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, iperf2_recv);
  return ERR_OK;
}

/**
  * @brief  initialize iperf server
  * @param  none
  * @retval none
  */
void iperf_init(void)
{
	void iperf3_init(void);
	iperf3_init();

	void iperf2_init(void);
	iperf2_init();
}

void iperf2_init(void)
{
    // Initialize TCP server
    struct tcp_pcb *tcp_pcb;
    lwiperf_state_tcp_t *s_tcp = (lwiperf_state_tcp_t *)LWIPERF_ALLOC(lwiperf_state_tcp_t);

    if (s_tcp == NULL) {
        printf("iperf_init TCP err_mem\r\n");
        return;
    }
    s_tcp->report_arg = NULL;
    tcp_pcb = tcp_new();
    tcp_bind(tcp_pcb, IP_ADDR_ANY, IPERF2_DEFAULT_PORT); // bind to iperf TCP port 5001
    tcp_pcb = tcp_listen(tcp_pcb);
    tcp_arg(tcp_pcb, s_tcp);
    tcp_accept(tcp_pcb, iperf2_accept);
    printf("iPerf2 TCP server listening on port %d\r\n", IPERF2_DEFAULT_PORT);

    // Initialize UDP server
    struct udp_pcb *udp_pcb;
    lwiperf_state_udp_t *s_udp = (lwiperf_state_udp_t *)LWIPERF_ALLOC(lwiperf_state_udp_t);

    if (s_udp == NULL) {
        printf("iperf_init UDP err_mem\r\n");
        return;
    }
    s_udp->time_started = 0;
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
    s_udp->time_started = sys_now();
    udp_recv(udp_pcb, iperf_udp_recv, s_udp);
    printf("iPerf2 UDP server listening on port %d\r\n", IPERF2_DEFAULT_PORT);
}

void iperf3_init(void)
{
    // Initialize TCP server
    struct tcp_pcb *tcp_pcb;
    iperf3_state_t *tcp_state = (iperf3_state_t *)LWIPERF_ALLOC(iperf3_state_t);

    if (tcp_state == NULL) {
        printf("[iPerf3] TCP server initialization failed - memory allocation error\r\n");
        return;
    }
    
    memset(tcp_state, 0, sizeof(iperf3_state_t));
    tcp_state->is_server = 1;
    tcp_state->protocol = 0; // TCP
    tcp_state->report_arg = NULL;
    printf("[iPerf3] init: creating TCP server on %u\r\n", IPERF3_DEFAULT_PORT);
    
    tcp_pcb = tcp_new();
    if (tcp_pcb == NULL) {
        LWIPERF_FREE(iperf3_state_t, tcp_state);
        printf("[iPerf3] TCP PCB creation failed\r\n");
        return;
    }
    
    if (tcp_bind(tcp_pcb, IP_ADDR_ANY, IPERF3_DEFAULT_PORT) != ERR_OK) {
        tcp_close(tcp_pcb);
        LWIPERF_FREE(iperf3_state_t, tcp_state);
        printf("[iPerf3] TCP bind failed\r\n");
        return;
    }
    
    tcp_pcb = tcp_listen(tcp_pcb);
    if (tcp_pcb == NULL) {
        LWIPERF_FREE(iperf3_state_t, tcp_state);
        printf("[iPerf3] TCP listen failed\r\n");
        return;
    }
    
    tcp_state->pcb.tcp = tcp_pcb;
    tcp_arg(tcp_pcb, tcp_state);
    tcp_accept(tcp_pcb, iperf3_accept);
    printf("[iPerf3] TCP server listening on port %d\r\n", IPERF3_DEFAULT_PORT);

    // Initialize UDP server
    struct udp_pcb *udp_pcb;
    iperf3_state_t *udp_state = (iperf3_state_t *)LWIPERF_ALLOC(iperf3_state_t);

    if (udp_state == NULL) {
        printf("[iPerf3] UDP server initialization failed - memory allocation error\r\n");
        return;
    }
    
    memset(udp_state, 0, sizeof(iperf3_state_t));
    udp_state->is_server = 1;
    udp_state->protocol = 1; // UDP
    udp_state->report_arg = NULL;
    udp_state->state = IPERF3_TEST_START;
    printf("[iPerf3] init: creating UDP server on %u\r\n", IPERF3_DEFAULT_PORT);

    udp_pcb = udp_new();
    if (udp_pcb == NULL) {
        LWIPERF_FREE(iperf3_state_t, udp_state);
        printf("[iPerf3] UDP PCB creation failed\r\n");
        return;
    }

    if (udp_bind(udp_pcb, IP_ADDR_ANY, IPERF3_DEFAULT_PORT) != ERR_OK) {
        udp_remove(udp_pcb);
        LWIPERF_FREE(iperf3_state_t, udp_state);
        printf("[iPerf3] UDP bind failed\r\n");
        return;
    }

    udp_state->pcb.udp = udp_pcb;
    udp_state->time_started = sys_now();
    udp_recv(udp_pcb, iperf3_udp_recv, udp_state);
    printf("[iPerf3] UDP server listening on port %d\r\n", IPERF3_DEFAULT_PORT);
    
//    printf("[iPerf3] Server initialized successfully\r\n");
}
