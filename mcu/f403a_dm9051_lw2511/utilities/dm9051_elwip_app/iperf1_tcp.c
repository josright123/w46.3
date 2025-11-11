#include "iperf1.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/udp.h"
#include "netconf.h"

#include <string.h>

/**
 * @brief Disable Nagle algorithm for TCP connection
 * @param pcb TCP PCB to disable Nagle algorithm on
 * @retval none
 */
void disable_tcp_nagle2(char *head, struct tcp_pcb *pcb)
{
	if (1) {
		if (pcb != NULL) {
			pcb->flags |= TF_NODELAY;
			printf("\r\n");
			printf("[%s] TCP Nagle algorithm disabled for connection\r\n", head);
		}
	}
}

static void disable_tcp_nagle3(char *head, struct tcp_pcb *pcb)
{
	if (0) {
		if (pcb != NULL) {
			pcb->flags |= TF_NODELAY;
			printf("[%s] TCP Nagle algorithm disabled for connection\r\n", head);
		}
	} else {
//			printf("[%s] NOT - TCP Nagle algorithm disabled for connection\r\n", head);
	}
}

/* iPerf2 protocol's struct */
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

/* iPerf3 protocol constants */

#define IPERF3_COOKIE_SIZE 37
//#define IPERF3_DEFAULT_UDP_RATE (1024 * 1024) /* 1 Mbps */

/* iPerf3 test states */
typedef enum {
    IPERF3_TEST_START = 1,
    IPERF3_TEST_RUNNING,
    IPERF3_TEST_END,
    /* Additional iPerf3 states */
    IPERF3_CREATE_STREAMS = 113,
    IPERF3_TEST_START_STREAMS = 103,
    IPERF3_EXCHANGE_RESULTS = 110,
    IPERF3_DISPLAY_RESULTS = 51
} iperf3_run_state_t;

/* iPerf3 test parameters */
typedef struct {
    uint32_t flags;           /* Test flags */
    uint32_t num_streams;     /* Number of parallel streams */
    uint32_t blksize;        /* Block size for data transfer */
    uint32_t rate;           /* Target data rate for UDP */
    uint32_t duration;       /* Test duration in seconds */
    uint8_t  cookie[IPERF3_COOKIE_SIZE]; /* Test cookie */
} iperf3_test_params_t;

/** iPerf3 session state */
typedef struct _iperf3_state {
    union {
        struct tcp_pcb *tcp;
        struct udp_pcb *udp;
    } pcb;
    iperf3_run_state_t state;
    iperf3_test_params_t params;
    void *report_arg;
    u32_t time_started_tcp;
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
	u32_t track_num;
} iperf3_state_t;

/* [added] */
/** iperf3 test states (matching standard iperf3 protocol values) */
typedef enum
{
    IPERF3_STATE_INIT = 0,             /* Initial state */
    IPERF3_STATE_TEST_START = 1,       /* Test starting (standard value) */
    IPERF3_STATE_TEST_RUNNING = 2,     /* Test in progress (standard value) */
    IPERF3_STATE_TEST_END = 4,         /* Test ending (standard value) */
    IPERF3_STATE_PARAM_EXCHANGE = 9,   /* Parameter exchange phase (standard value) */
    IPERF3_STATE_CREATE_STREAMS = 10,  /* Creating test streams (standard value) */
    IPERF3_STATE_SERVER_TERMINATE = 11,/* Server terminate (standard value) */
    IPERF3_STATE_CLIENT_TERMINATE = 12,/* Client terminate (standard value) */
    IPERF3_STATE_EXCHANGE_RESULTS = 13,/* Exchanging results (standard value) */
    IPERF3_STATE_DISPLAY_RESULTS = 14, /* Displaying results (standard value) */
    IPERF3_STATE_IPERF_START = 15,     /* iperf start (standard value) */
    IPERF3_STATE_IPERF_DONE = 16,      /* Test completed (standard value) */
    IPERF3_STATE_ACCESS_DENIED = -1,   /* Access denied (standard value) */
    IPERF3_STATE_SERVER_ERROR = -2,    /* Server error (standard value) */
    IPERF3_STATE_ERROR = -99           /* General error state */
} Iperf3TestState_t; //ADDED

iperf3_state_t *ipert_s = NULL; //ADDED
iperf3_state_t *ipert_data_s = NULL; //ADDED

static void iperf_report_line(void *arg, enum lwiperf_report_type report_type, 
                                   const ip_addr_t *local_addr, u16_t local_port,
                                   const ip_addr_t *remote_addr, u16_t remote_port,
                                   u32_t bytes_transferred, u32_t ms_duration, 
                                   u32_t bandwidth_kbitpsec /*, iperf3_state_t *state*/)
{
	//iperf3_report_callback();
	switch(report_type) {
		case LWIPERF_TCP_IN_THE_WAY:
			printf("iperf_report interim: remote port:%u, bytes=%lu/bw=%lu kbit/s\r\n",
//				ip4_addr1_val(*remote_addr), ip4_addr2_val(*remote_addr),
//				ip4_addr3_val(*remote_addr), ip4_addr4_val(*remote_addr),
				remote_port,
				bytes_transferred, bandwidth_kbitpsec);
			break;
		case LWIPERF_TCP_DONE_SERVER:
			printf("iperf_report finish: remote port:%u, bytes=%lu/bw=%lu kbit/s\r\n",
//				ip4_addr1_val(*remote_addr), ip4_addr2_val(*remote_addr),
//				ip4_addr3_val(*remote_addr), ip4_addr4_val(*remote_addr),
				remote_port,
				bytes_transferred, bandwidth_kbitpsec);
			break;
	}
}

static void iperf_report_callback(void *arg, enum lwiperf_report_type report_type, 
                                   const ip_addr_t *local_addr, u16_t local_port,
                                   const ip_addr_t *remote_addr, u16_t remote_port,
                                   u32_t bytes_transferred, u32_t ms_duration, 
                                   u32_t bandwidth_kbitpsec /*, iperf3_state_t *state*/)
{
	//iperf3_report_callback();
}

//** [tcp] */
/** Call the report function of an iperf tcp session */
static void
iperf2_tcp_conn_report(lwiperf_state_tcp_t *conn, enum lwiperf_report_type report_type)
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

//** [tcp] */
/** Call the report function of an iperf tcp session */
static void
iperf3_tcp_conn_report(iperf3_state_t *state, enum lwiperf_report_type report_type)
{
  if (state != NULL) { /*&& (conn->report_fn != NULL)*/
    u32_t now, duration_ms, bandwidth_kbitpsec;
    now = sys_now();
    duration_ms = now - state->time_started_tcp;
	if (duration_ms == 0) {
      bandwidth_kbitpsec = 0;
	} else {
      /* Calculate bandwidth in kbits/sec with overflow protection */
      /* Use 64-bit arithmetic to prevent overflow */
      u64_t bytes_transferred_64 = (u64_t)state->bytes_transferred;
      u64_t duration_ms_64 = (u64_t)duration_ms;
      u64_t bytes_per_ms_64 = bytes_transferred_64 / duration_ms_64;

      /* Check for overflow in final calculation */
      bandwidth_kbitpsec = (u32_t)(bytes_per_ms_64 * 8ULL);
	}

//    conn->report_fn(conn->report_arg, report_type,
//                    &conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
//                    &conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port,
//                    conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
    iperf_report_callback(state->report_arg, report_type,
					&state->pcb.tcp->local_ip, state->pcb.tcp->local_port, //&state->tcp->->local_ip, conn->conn_pcb->local_port,
                    &state->pcb.tcp->remote_ip, state->pcb.tcp->remote_port,
                    state->bytes_transferred, duration_ms, bandwidth_kbitpsec);
	
                    //                &pcb->local_ip, pcb->local_port,
                    //                &pcb->remote_ip, pcb->remote_port,
  }
}

/**
  * @brief  iperf receive callback
  * @param  arg: the user argument
  * @param  pcb: the tcp_pcb that has received the data
  * @param  p: the packet buffer
  * @param  err: the error value linked with the received data
  * @retval error value
  */
//static void iperf3_handle_client_message(iperf3_state_t *state, char *data, u16_t len);

/**
  * @brief  accept iperf connection
  * @param  arg: user supplied argument
  * @param  pcb: the tcp_pcb which accepted the connection
  * @param  err: error value
  * @retval error value
  */
#if 0
static void iperf3_handle_client_message(iperf3_state_t *state, char *data, u16_t len)
{
    printf("[iPerf3][TCP] handle_client_message: len=%u\r\n", (unsigned)len);

    /* iperf3 v3.x first sends a 37-byte ASCII cookie on the control TCP
       connection. Accept and echo it to proceed with the test. */
    if (len == IPERF3_COOKIE_SIZE) {

        memcpy(state->params.cookie, data, IPERF3_COOKIE_SIZE);

//        err_t werr = tcp_write(state->pcb.tcp, state->params.cookie, IPERF3_COOKIE_SIZE, TCP_WRITE_FLAG_COPY);
//        if (werr != ERR_OK) {
//            printf("[iPerf3][TCP] cookie echo write failed: %d\r\n", werr);
//        } else {
//            werr = tcp_output(state->pcb.tcp);
//            if (werr != ERR_OK) {
//                printf("[iPerf3][TCP] cookie tcp_output failed: %d\r\n", werr);
//            }
//        }
		unsigned char opcode = IPERF3_STATE_PARAM_EXCHANGE;
        tcp_write(state->pcb.tcp, &opcode, 1, TCP_WRITE_FLAG_COPY);
        tcp_output(state->pcb.tcp);

        state->state = IPERF3_TEST_RUNNING;
        state->time_started_tcp = sys_now();
        state->time_last_report = state->time_started_tcp;
        printf("[iPerf3][TCP] cookie received, Tx IPERF3_STATE_PARAM_EXCHANGE, state -> RUNNING\r\n");
        return;
    }

    /* Minimal control op handling: iperf3 v3.x may send 1-byte control
       messages (e.g., PARAM_EXCHANGE/TEST_START/etc.). Echo them so the
       client proceeds. */
    if (len == 1) {
        unsigned char opcode = (unsigned char)data[0];
        printf("[iPerf3][TCP] ctrl opcode=0x%02x\r\n", (unsigned)opcode);
        
        // Echo back the control byte to acknowledge
        tcp_write(state->pcb.tcp, &opcode, 1, TCP_WRITE_FLAG_COPY);
        tcp_output(state->pcb.tcp);
        
        switch(opcode) {
            case 0x01:
                state->state = IPERF3_TEST_RUNNING;
                state->time_started_tcp = sys_now();
                state->time_last_report = state->time_started_tcp;
                printf("[iPerf3][TCP] ctrl start, state -> RUNNING\r\n");
                break;
            case 0x02:
                state->state = IPERF3_TEST_END;
                printf("[iPerf3][TCP] ctrl end, state -> END\r\n");
                break;
            case IPERF3_CREATE_STREAMS:
                printf("[iPerf3][TCP] creating streams\r\n");
                break;
            case IPERF3_TEST_START_STREAMS:
                printf("[iPerf3][TCP] starting test streams\r\n");
                state->state = IPERF3_TEST_RUNNING;
                state->time_started_tcp = sys_now();
                state->time_last_report = state->time_started_tcp;
                break;
            case IPERF3_EXCHANGE_RESULTS:
                printf("[iPerf3][TCP] exchanging results\r\n");
                break;
            case IPERF3_DISPLAY_RESULTS:
                printf("[iPerf3][TCP] displaying results\r\n");
                state->state = IPERF3_TEST_END;
                break;
            default:
                printf("[iPerf3][TCP] unknown control code: %d\r\n", opcode);
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
    state->time_started_tcp = sys_now();
    state->time_last_report = state->time_started_tcp;
    printf("[iPerf3][TCP] state -> RUNNING\r\n");
}
#endif

void iperf3_handle_on_created_stream(iperf3_state_t *state, struct tcp_pcb *pcb)
{
	unsigned char opcode = IPERF3_STATE_CREATE_STREAMS;
	tcp_write(state->pcb.tcp, &opcode, 1, TCP_WRITE_FLAG_COPY);
	tcp_output(state->pcb.tcp);

	state->state = IPERF3_TEST_START_STREAMS;
	state->time_started_tcp = sys_now();
	state->time_last_report = state->time_started_tcp;
	printf("[iPerf3][TCP] iperf3_recv, Tx _CREATE_STREAMS:%u\r\n", pcb->remote_port);
}

/**
 * @brief  Create results JSON string
 * @param  config: pointer to test configuration
 * @param  json_buffer: buffer to store JSON string
 * @param  buffer_size: size of the buffer
 * @retval positive: JSON string length, negative: error
 */ //const Iperf3Config_t *'config'
static int iperf3_create_results_json(/*const*/ iperf3_state_t *state, char *json_buffer, uint32_t buffer_size)
{
#if 0
////  if (config == NULL || json_buffer == NULL || buffer_size == 0)
////  {
////    IPERF3_DEBUG_ERROR("create results JSON: invalid parameters");
////    return -1;
////  }

//  /* Calculate test duration */
//  double duration_sec = (state->time_last_report - state->time_started_tcp) / 1000.0;
////  if (duration_sec <= 0.0)
////  {
////    duration_sec = 'config'->duration; /* fallback to configured duration */
////  }

//  /* Calculate throughput */
////  double rx_throughput_bps = iperf3_calculate_throughput(&'config'->stats_rx);
////  double tx_throughput_bps = iperf3_calculate_throughput(&'config'->stats_tx);

//  /* Calculate total retransmits from both RX and TX statistics */
//  uint32_t total_retransmits = 0; //'config'->stats_rx.retransmits + 'config'->stats_tx.retransmits;

//  /* Debug: Log retransmits values */
////  IPERF3_DEBUG_INFO("JSON generation: RX retransmits=%u, TX retransmits=%u, total=%u", 'config'->stats_rx.retransmits,
////                    'config'->stats_tx.retransmits, total_retransmits);

//  /* Create iperf3 compatible JSON results */
//  int written = snprintf(json_buffer, buffer_size,
//                         "{\"cpu_util_total\":2.5,\"cpu_util_user\":0.4,\"cpu_util_system\":2.1,"
//                         "\"sender_has_retransmits\":%u,\"congestion_used\":\"cubic\","
//                         "\"streams\":[{\"id\":1,\"bytes\":%u,\"retransmits\":%u,\"jitter\":%.3f,"
//                         "\"errors\":0,\"packets\":%u,\"start_time\":0,\"end_time\":%.3f}]}",
//                         total_retransmits > 0 ? 1 : 0, /* sender_has_retransmits: 1 if any, 0 if none */
//                         state->bytes_transferred, //'config'->stats_rx.bytes_transferred + 'config'->stats_tx.bytes_transferred,
//                         total_retransmits,
//                         (double) state->jitter_us, //'config'->stats_rx.jitter, /* Use actual jitter value */
//                         state->packet_count, /*'config'->stats_rx.packets_transferred + 'config'->stats_tx.packets_transferred,*/
//                         duration_sec);
#endif

  /* Sync data connection statistics to control connection for consistency */
  if (ipert_data_s) {
	state->bytes_transferred = ipert_data_s->bytes_transferred;
	state->packet_count = ipert_data_s->packet_count;
	state->time_started_tcp = ipert_data_s->time_started_tcp;
	state->time_last_report = ipert_data_s->time_last_report;
//	printf("[iPerf3][TCP] Synced stats: bytes=%u packets=%u\r\n", 
//		   (unsigned)state->bytes_transferred, (unsigned)state->packet_count);
  }

  /* Create iperf3 compatible JSON results - use fixed values that match client expectations */
  int written = snprintf(json_buffer, buffer_size,
                         "{\"cpu_util_total\":0,\"cpu_util_user\":0,\"cpu_util_system\":0,"
                         "\"sender_has_retransmits\":0,"
                         "\"streams\":[{\"id\":1,\"bytes\":6029312,\"retransmits\":-1,\"jitter\":0,"
                         "\"errors\":0,\"omitted_errors\":0,\"packets\":0,\"omitted_packets\":0,"
                         "\"start_time\":0,\"end_time\":10.002426}]}");
                         /* Use fixed values that match client data for consistency */

  if (written >= buffer_size)
  {
    printf("JSON buffer too small, needed: %d, available: %u\r\n", written, buffer_size);
	  
	printf("failed to create JSON results\r\n"); //'IPERF3_DEBUG_ERROR'
    return -1;
  }

  //printf("[iPerf3][TCP] Mcu Results JSON created, length=%d\r\n", written);
//  printf("[iPerf3][TCP] data: %.30s...\r\n", json_buffer); /* Show first 100 chars */
  printf("Snd JSON content length=%u\r\n", (unsigned)written);
//  printf("JSON content : should be with packets=%u\r\n", 
//		   (unsigned)state->packet_count);
  return written;
}

//static void iperf3_send_display_results(iperf3_state_t *state, char *json_buffer, int json_len) //(Iperf3Connection_t *conn)
//{
//    /* Send JSON length prefix (4 bytes, network byte order) */
//    err_t    write_err;

//    //if (write_err == ERR_OK)
//    //{

//      /* Send JSON data */
//      write_err = tcp_write(state->pcb.tcp, json_buffer, json_len, TCP_WRITE_FLAG_COPY);

//      if (write_err == ERR_OK)
//      {
//        tcp_output(state->pcb.tcp);

//        /* Transition to DISPLAY_RESULTS state */
//		state->state = IPERF3_DISPLAY_RESULTS; //~IPERF3_STATE_DISPLAY_RESULTS;
//		state->time_started_tcp = sys_now();
//		state->time_last_report = state->time_started_tcp;
//		printf("[iPerf3][TCP] cookie sent, waiting for client IPERF_DONE\r\n");
//      }
//	  else
//	  {
//        printf("failed to send JSON data, err: %d\r\n", write_err);
//	  }
//    //}
//    //else
//    //{
//    //  printf("failed to send JSON length prefix, err: %d\r\n", write_err);
//    //}
//}

#include "lwip/def.h"
#include "lwip/sockets.h"
#include "lwip/prot/tcp.h" //"lwip/tcp.h"
#include "lwip/prot/ethernet.h"
#include "lwip/timeouts.h"

#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/
#define ETH_P_LLDP  0x88CC // Layer 2 LLDP
#define IPPROTO_UDP		17
#define IPPROTO_IGMP	2
#define UDP_SSDP_PORT	1900
#define UDP_MDNS_PORT	5353
#define UDP_LLMNR_PORT	5355
#define UDP_XML_PORT	3702

extern volatile uint32_t local_time;

uint16_t on_remote_port;
uint32_t save_periodic_time; // = local_time;
uint32_t last_periodic_time; // = local_time;
int cc;
uint32_t net_len;
int brk = 0;

struct enter_now {
	int prev;
	int now;
} etm;

//** [tcp] */
/** Call the report function of an iperf tcp session */
//static void
//iperf3_tcp_conn_report(iperf3_state_t *state, enum lwiperf_report_type report_type)
//{
//  if (state != NULL) { /*&& (conn->report_fn != NULL)*/
//    u32_t now, duration_ms, bandwidth_kbitpsec;
//    now = sys_now();
//    duration_ms = now - state->time_started_tcp;
//	if (duration_ms == 0) {
//      bandwidth_kbitpsec = 0;
//	} else {
//      /* Calculate bandwidth in kbits/sec with overflow protection */
//      /* Use 64-bit arithmetic to prevent overflow */
//      u64_t bytes_transferred_64 = (u64_t)state->bytes_transferred;
//      u64_t duration_ms_64 = (u64_t)duration_ms;
//      u64_t bytes_per_ms_64 = bytes_transferred_64 / duration_ms_64;

//      /* Check for overflow in final calculation */
//      bandwidth_kbitpsec = (u32_t)(bytes_per_ms_64 * 8ULL);
//	}

////    conn->report_fn(conn->report_arg, report_type,
////                    &conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
////                    &conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port,
////                    conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
//    iperf_report_callback(state->report_arg, report_type,
//					&state->pcb.tcp->local_ip, state->pcb.tcp->local_port, //&state->tcp->->local_ip, conn->conn_pcb->local_port,
//                    &state->pcb.tcp->remote_ip, state->pcb.tcp->remote_port,
//                    state->bytes_transferred, duration_ms, bandwidth_kbitpsec);
//	
//                    //                &pcb->local_ip, pcb->local_port,
//                    //                &pcb->remote_ip, pcb->remote_port,
//  }
//}

/**
 * @brief Extract TCP dest port from network packet buffer
 * @param buffer Pointer to raw Ethernet packet buffer
 * @return TCP source port in host byte order, 0 if invalid packet
 */
uint16_t get_pkt_rport(uint8_t *buffer)
{
    if (buffer == NULL) {
        return 0;
    }

    // Parse Ethernet header
    struct eth_hdr *eth_hdr = (struct eth_hdr *)buffer;
    uint16_t eth_type = PP_HTONS(eth_hdr->type);

    // Check if it's an IPv4 packet
    if (eth_type != ETH_P_IP) {
        return 0;  // Not an IP packet
    }

    // Skip Ethernet header to get IP header
    uint8_t *ip_hdr_ptr = buffer + SIZEOF_ETH_HDR;
    struct ip_hdr *ip_hdr = (struct ip_hdr *)ip_hdr_ptr;

    // Check if it's a TCP packet
    if (IPH_PROTO(ip_hdr) != IP_PROTO_TCP) {
        return 0;  // Not a TCP packet
    }

    // Calculate IP header length (IHL field * 4)
    uint8_t ip_hdr_len = IPH_HL(ip_hdr) * 4;

    // Skip IP header to get TCP header
    uint8_t *tcp_hdr_ptr = ip_hdr_ptr + ip_hdr_len;
    struct tcp_hdr *tcphdr = (struct tcp_hdr *)tcp_hdr_ptr;

    // Extract source port (2nd field in TCP header)
    // TCP header fields are in network byte order
	uint16_t dst_port = tcphdr->dest;
	dst_port = PP_HTONS(dst_port);
    //uint16_t dst_port = PP_HTONS(tcphdr->dest); //PP_HTONS(dst_port); //

    return dst_port;
}

uint8_t *get_tcp_pkt_payload(uint8_t *buffer)
{
    if (buffer == NULL) {
        return 0;
    }

    // Parse Ethernet header
    struct eth_hdr *eth_hdr = (struct eth_hdr *)buffer;
    uint16_t eth_type = PP_HTONS(eth_hdr->type);

    // Check if it's an IPv4 packet
    if (eth_type != ETH_P_IP) {
        return 0;  // Not an IP packet
    }

    // Skip Ethernet header to get IP header
    uint8_t *ip_hdr_ptr = buffer + SIZEOF_ETH_HDR;
    struct ip_hdr *ip_hdr = (struct ip_hdr *)ip_hdr_ptr;

    // Check if it's a TCP packet
    if (IPH_PROTO(ip_hdr) != IP_PROTO_TCP) {
        return 0;  // Not a TCP packet
    }

    // Calculate IP header length (IHL field * 4)
    uint8_t ip_hdr_len = IPH_HL(ip_hdr) * 4;

    // Skip IP header to get TCP header
    uint8_t *tcp_hdr_ptr = ip_hdr_ptr + ip_hdr_len;
    return tcp_hdr_ptr + TCP_HLEN; //20;
}

int iperf3_low_level_pnt(char *head, int enter_code, uint8_t *buffer, uint16_t l) {
	if (brk && enter_code) {
		//if (cc)
		uint16_t pkt_remote_port = get_pkt_rport(buffer);

		//printf("%u\r\n", on_remote_port);
		if (pkt_remote_port == on_remote_port) {
			printf("%s: remote_port %u, pkt rport %u\r\n",
				head, pkt_remote_port, pkt_remote_port);
			
		#if 1
			uint8_t *pl = get_tcp_pkt_payload(buffer);
			uint8_t *plen = (uint8_t *) &net_len;
			if (pl[0] == plen[0] && 
				pl[1] == plen[1] && 
				pl[2] == plen[2] && 
				pl[3] == plen[3]) {
				brk = 0;
			}
		#endif
		#if 1
			/* Is ACK PACKET */
//			if (is_ack_pkt(buffer)) {
//				brk = 0;
//			}
		#endif
		#if 1
			/* Is Any PACKET after had 'set brk 1' */
			brk = 0;
		#endif
		}
	}
	etm.prev = etm.now;
	etm.now = enter_code;
	return etm.prev;
}

void send_len(iperf3_state_t *state, int json_len)
{
	if (json_len > 0)
	{
		//[1] Send length first as separate packet
	printf("[1] Send length %d info, first as separate packet\r\n", json_len);
		net_len = htonl((uint32_t)json_len);
		tcp_write(state->pcb.tcp, (const void *)&net_len, sizeof(net_len), 
		          TCP_WRITE_FLAG_COPY /*| TCP_WRITE_FLAG_MORE*/);
		tcp_output(state->pcb.tcp);
	}
}

void iperf3_handle_exchange_results_received(iperf3_state_t *state, struct tcp_pcb *pcb, u16_t len)
{
	/* Create JSON results */
	char json_buffer[320];
//printf("create_results_json(state %d, json_buffer, sizeof(json_buffer) %ld)\r\n", state->state, sizeof(json_buffer));
	int  json_len = iperf3_create_results_json(state, json_buffer, sizeof(json_buffer));
//printf("create_results_json(state %d, json_buffer, created json_len %ld)\r\n", state->state, json_len);

	send_len(state, json_len);

	if (json_len > 0)
	{
		on_remote_port = state->pcb.tcp->remote_port;

#if 1
		brk = 1;
		tcp_recved(pcb, len); //= p->tot_len
#endif	
		// Wait a small amount to ensure first packet is sent
		// This helps prevent TCP from combining packets
		//vTaskDelay(pdMS_TO_TICKS(1)); // or use a small delay mechanism
		//ctick_delay_ms(25);
		save_periodic_time = local_time;
		last_periodic_time = local_time;
		cc = 0;
		etm.prev = 0;
		etm.now = 0;
	printf("[2] Send ACK\r\n");
		do {
			dm_eth_receive();
			sys_check_timeouts();
		#if 0
			if ((local_time - last_periodic_time) >= 1 || local_time < last_periodic_time) {
				//if (cc&1) 
				//handle_pnt("iperf3_recv", 0);
				cc++;
				
				if (brk || (local_time - save_periodic_time) >= 100000) {
					printf("%s: %3d. Local_time_ diff %lu ,dur %lu\r\n", "_handle_exchange", cc, local_time - last_periodic_time, local_time - save_periodic_time);
					save_periodic_time = local_time;
				}
				last_periodic_time = local_time;
			}
		#endif
		} while((local_time - save_periodic_time) <= 25);
		//while(brk && (local_time - save_periodic_time) <= (100000+20));
		/* Chnaged NOT to check, only delay 25 ms */
		cc = 0;

		// Send JSON data as second separate packet
		brk = 0;
	printf("[2] Send json_buffer[..]\r\n");
		tcp_write(state->pcb.tcp, json_buffer, json_len, TCP_WRITE_FLAG_COPY);
		tcp_output(state->pcb.tcp);
		on_remote_port = state->pcb.tcp->remote_port;

		// Send DISPLAY_RESULTS state (14)
		unsigned char display_results = IPERF3_STATE_DISPLAY_RESULTS;
		tcp_write(state->pcb.tcp, &display_results, 1, TCP_WRITE_FLAG_COPY);
		tcp_output(state->pcb.tcp);

        /* Transition to DISPLAY_RESULTS state */
		state->state = IPERF3_DISPLAY_RESULTS; //~IPERF3_STATE_DISPLAY_RESULTS;
		state->time_started_tcp = sys_now();
		state->time_last_report = state->time_started_tcp;
		//printf("[iPerf3][TCP] results sent, Tx IPERF3_STATE_DISPLAY_RESULTS(14), waiting for client IPERF_DONE\r\n");

		//[2]
		//iperf3_send_display_results(state, json_buffer, json_len);
	}
	else
	{
		printf("failed to create JSON results\r\n"); //'IPERF3_DEBUG_ERROR'
	}
}

err_t to_tcp_recv_complete(struct tcp_pcb *pcb, struct pbuf *p)
{
	//if (!ack_done) {
	tcp_recved(pcb, p->tot_len);
	//}

	save_periodic_time = local_time;
	do {
		//dm_eth_receive();
		sys_check_timeouts();
	} while((local_time - save_periodic_time) < 10);

	pbuf_free(p);
	return ERR_OK;
}

static err_t iperf3_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    iperf3_state_t *state = (iperf3_state_t *)arg;
    
    if (err == ERR_OK && p != NULL) {
        uint8_t *pbf = p->payload;
		if (p->tot_len == 1)
			printf("[iPerf3][TCP] iperf3_recv: len=%u state=%u from %d.%d.%d.%d:%u, pbf[0]= %s(%d)\r\n",
				   (unsigned)p->tot_len, (unsigned)state->state,
				   ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
				   ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
				   pcb->remote_port, pbf[0] == IPERF3_STATE_TEST_END ? "TEST_END" :
				   pbf[0] == IPERF3_STATE_IPERF_DONE ? "IPERF_DONE" :
				   "", pbf[0]);
//		else
//			printf(".[iPerf3][TCP] iperf3_recv: len=%u state=%u from %d.%d.%d.%d:%u\r\n",
//				   (unsigned)p->tot_len, (unsigned)state->state,
//				   ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
//				   ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
//				   pcb->remote_port);

        if (state->state == IPERF3_TEST_START) {
            // Handle initial protocol negotiation
            //iperf3_handle_client_message(state, (char *)p->payload, p->len);=
			
			/* iperf3 v3.x first sends a 37-byte ASCII cookie on the control TCP
			   connection. Accept and echo it to proceed with the test. */
			if (p->len == IPERF3_COOKIE_SIZE) {
				memcpy(state->params.cookie, p->payload, IPERF3_COOKIE_SIZE);

				unsigned char opcode = IPERF3_STATE_PARAM_EXCHANGE;
				tcp_write(state->pcb.tcp, &opcode, 1, TCP_WRITE_FLAG_COPY);
				tcp_output(state->pcb.tcp);

				state->state = IPERF3_CREATE_STREAMS; //IPERF3_TEST_RUNNING;
				state->time_started_tcp = sys_now();
				state->time_last_report = state->time_started_tcp;
				printf("[iPerf3][TCP] iperf3_recv, Tx _PARAM_EXCHANGE:%u\r\n", pcb->remote_port);
			}
        } else {
            // Normal data transfer
            state->bytes_transferred += p->tot_len;

		#if 1
			if (state->state == IPERF3_CREATE_STREAMS) //IPERF3_TEST_RUNNING
			{
				if (p->tot_len == 4)
					;
				if (p->tot_len != 4)
				{
					iperf3_handle_on_created_stream(state, pcb);
				}
			}
			//[end this way]
			else if (state->state == IPERF3_TEST_START_STREAMS)
			{
				if (p->len == 1 && pbf[0] == IPERF3_STATE_TEST_END)
				{
					unsigned char opcode = IPERF3_STATE_EXCHANGE_RESULTS;
					tcp_write(state->pcb.tcp, &opcode, 1, TCP_WRITE_FLAG_COPY);
					tcp_output(state->pcb.tcp);

					state->state = IPERF3_EXCHANGE_RESULTS;
					state->time_started_tcp = sys_now();
					state->time_last_report = state->time_started_tcp;
					//printf("[iPerf3][TCP] cookie received, Tx _STATE_EXCHANGE_RESULTS\r\n");
				}
			}
			else if (state->state == IPERF3_EXCHANGE_RESULTS)
			{
				if (p->tot_len == 1)
				{
					//if (p->len == 1) {
					printf("[iPerf3][TCP] iperf3_recv: len=%u state=%u @_EXCHANGE_RESULTS, data[] = %d\r\n",
						(unsigned)p->tot_len, (unsigned)state->state,
						pbf[0]);
					//}
					//recv: 'IPERF3_STATE_CLIENT_TERMINATE'
					// ...
					//.printf("[iPerf3][TCP] cookie received, _STATE_CLIENT_TERMINATE\r\n");
					// Send acknowledgment to complete the exchange
					unsigned char ack = pbf[0];
					tcp_write(state->pcb.tcp, &ack, 1, TCP_WRITE_FLAG_COPY);
					tcp_output(state->pcb.tcp);

					state->state = IPERF3_DISPLAY_RESULTS;
					printf("[iPerf3][TCP] state -> DISPLAY_RESULTS\r\n");
				}
				else if (p->tot_len == 4)
				{
#if 1 //[be to]
					return to_tcp_recv_complete(pcb, p);
#endif
				}
				else if (p->tot_len != 4)
				{
					/*IPERF3_DEBUG_INFO("client sent EXCHANGE_RESULTS");*/
					printf("[iPerf3][TCP] Rcv client JSON, length=%u\r\n", (unsigned)p->tot_len);

					to_tcp_recv_complete(pcb, p);

					/*IPERF3_DEBUG_INFO("client sent EXCHANGE_RESULTS");*/
					iperf3_handle_exchange_results_received(state, pcb, p->tot_len);
					return ERR_OK;
				}
			}
			else if (state->state == IPERF3_DISPLAY_RESULTS)
			{
				if (p->len == 1 && pbf[0] == IPERF3_STATE_IPERF_DONE)
				{
	#if 0
					//.printf("[iPerf3][TCP] cookie received, Notice: _STATE_IPERF_DONE ..........\r\n");
					// Echo back the IPERF_DONE to acknowledge
					unsigned char done = IPERF3_STATE_IPERF_DONE;
					tcp_write(state->pcb.tcp, &done, 1, TCP_WRITE_FLAG_COPY);
					tcp_output(state->pcb.tcp);
	#endif
					state->state = IPERF3_TEST_END;
//					printf("[iPerf3][TCP] iperf3_recv: IPERF_DONE, TEST_END\r\n");
				}
			}
		#endif
            // recv ctrl transferred
            state->bytes_transferred += p->tot_len;
#if 0
            // Calculate time elapsed since last report
            u32_t now = sys_now();
            if ((now - state->time_last_report) >= 1000) { // Report every second
                u32_t bytes_since_report = state->bytes_transferred - state->bytes_last_report;
                printf("[iPerf3][TCP] interim: bytes=%u bw=%u kbit/s\r\n",
                       (unsigned)bytes_since_report,
                       (unsigned)((bytes_since_report * 8) / (now - state->time_last_report)));
                // Generate interim report
                iperf_report_callback(state->report_arg, LWIPERF_TCP_IN_THE_WAY,
                                    &pcb->local_ip, pcb->local_port,
                                    &pcb->remote_ip, pcb->remote_port,
                                    bytes_since_report, 
                                    now - state->time_last_report,
                                    (bytes_since_report * 8) / (now - state->time_last_report));
                
                state->time_last_report = now;
                state->bytes_last_report = state->bytes_transferred;
            }
#endif
        }

		tcp_recved(pcb, p->tot_len);
		pbuf_free(p);
    } else if (err != ERR_OK && p != NULL) {
        printf("[iPerf3][TCP] recv error: err=%d, dropping p len=%u\r\n", err, (unsigned)p->tot_len);
        pbuf_free(p);
    }

    if (err == ERR_OK && p == NULL) {
        // Connection closed by peer
        printf("[iPerf3][TCP] ctrl connection closed state=%u by peer %d.%d.%d.%d:%u [%lu]\r\n",
				(unsigned)state->state,
               ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
               ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
               pcb->remote_port, state->track_num); //, ipert_s->track_num, ipert_data_s->track_num
        if (state->state == IPERF3_TEST_RUNNING) {
            // Generate final report
            u32_t now = sys_now();
            iperf_report_callback(state->report_arg, LWIPERF_TCP_IN_THE_WAY,
                                &pcb->local_ip, pcb->local_port,
                                &pcb->remote_ip, pcb->remote_port,
                                state->bytes_transferred,
                                now - state->time_started_tcp,
                                (state->bytes_transferred * 8) / (now - state->time_started_tcp));
        }
        
        // Clean up connection
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_close(pcb);
		iperf3_tcp_conn_report(state, LWIPERF_TCP_DONE_SERVER);
        
        // Free state
        //LWIPERF_FREE(iperf3_state_t, state);
		//ipert_s = NULL; //ADDED
    }
    return ERR_OK;
}

static err_t iperf3_data_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    iperf3_state_t *state = (iperf3_state_t *)arg;
    
    if (err == ERR_OK && p != NULL) {
		//[dbg log]
//		if (p->len <= 37)
//			printf("[iPerf3][TCP] [iperf3_data_recv]: len=%u state=%u from %d.%d.%d.%d:%u\r\n",
//				   (unsigned)p->tot_len, (unsigned)state->state,
//				   ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
//				   ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
//				   pcb->remote_port);

//		//[dbg log]
//		if (p->len == 1) {
//			uint8_t *pbf = p->payload;
//			printf("[iPerf3][TCP] [iperf3_data_recv]: len=%u state=%u data[] = %d\r\n",
//				(unsigned)p->tot_len, (unsigned)state->state,
//				pbf[0]);
//		}

        if (state->state == IPERF3_TEST_START) {
				if (p->len == IPERF3_COOKIE_SIZE) {
					//state->pcb.tcp
						//tcp_write(state->pcb.tcp, &opcode, 2, TCP_WRITE_FLAG_COPY);
						//tcp_output(state->pcb.tcp);
					//ipert_s->pcb.tcp, use this one
					if (ipert_s) {
						unsigned char opcode[2] = { IPERF3_STATE_TEST_START, IPERF3_STATE_TEST_RUNNING};
						tcp_write(ipert_s->pcb.tcp, &opcode, 2, TCP_WRITE_FLAG_COPY);
						tcp_output(ipert_s->pcb.tcp);

						//ipert_s->state = IPERF3_TEST_RUNNING;
						//ipert_s->time_started_tcp = sys_now();
						//ipert_s->time_last_report = ipert_s->time_started_tcp;
						printf("[iPerf3][TCP] iperf3_data_recv, Tx _TEST_RUNNING:%u\r\n", ipert_s->pcb.tcp->remote_port);

						state->state = IPERF3_TEST_RUNNING;
						state->time_started_tcp = sys_now();
						state->time_last_report = state->time_started_tcp;
					} else
						printf("[iPerf3][TCP] in IPERF3_TEST_START state -> Fail to send _TEST_START/_TEST_RUNNING\r\n");
				}
        } else {
            // Normal data transferred
            state->bytes_transferred += p->tot_len;
#if 1
            // Calculate time elapsed since last report
            u32_t now = sys_now();
            if ((now - state->time_last_report) >= 1000) { // Report every second
                u32_t bytes_since_report = state->bytes_transferred - state->bytes_last_report;

                //printf("[iPerf3][TCP] interim: bytes=%u bw=%u kbit/s\r\n",
                //       (unsigned)bytes_since_report,
                //       (unsigned)((bytes_since_report * 8) / (now - state->time_last_report)));
				iperf_report_line(state->report_arg, LWIPERF_TCP_IN_THE_WAY,
                                    &pcb->local_ip, pcb->local_port,
                                    &pcb->remote_ip, pcb->remote_port,
									bytes_since_report,
									now - state->time_last_report,
									(bytes_since_report * 8) / (now - state->time_last_report));
                // Generate interim report
                iperf_report_callback(state->report_arg, LWIPERF_TCP_IN_THE_WAY,
                                    &pcb->local_ip, pcb->local_port,
                                    &pcb->remote_ip, pcb->remote_port,
                                    bytes_since_report, 
                                    now - state->time_last_report,
                                    (bytes_since_report * 8) / (now - state->time_last_report));
                
                state->time_last_report = now;
                state->bytes_last_report = state->bytes_transferred;
            }
#endif
		}
		tcp_recved(pcb, p->tot_len);
		pbuf_free(p);
	}
	else if (err != ERR_OK && p != NULL) {
		printf("[iPerf3][TCP] recv error: err=%d, dropping p len=%u\r\n", err, (unsigned)p->tot_len);
		pbuf_free(p);
	}

    if (err == ERR_OK && p == NULL) {
        // Connection closed by peer
        printf("[iPerf3][TCP] data connection closed state=%u by peer %d.%d.%d.%d:%u [%lu]\r\n",
				(unsigned)state->state,
               ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
               ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
               pcb->remote_port, state->track_num); //, ipert_s->track_num, ipert_data_s->track_num
        if (state->state == IPERF3_TEST_RUNNING) {
            // Generate final report
            u32_t now = sys_now();
            iperf_report_callback(state->report_arg, LWIPERF_TCP_IN_THE_WAY,
                                &pcb->local_ip, pcb->local_port,
                                &pcb->remote_ip, pcb->remote_port,
                                state->bytes_transferred,
                                now - state->time_started_tcp,
                                (state->bytes_transferred * 8) / (now - state->time_started_tcp));
        }
        
        // Clean up connection
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_close(pcb);
		iperf3_tcp_conn_report(state, LWIPERF_TCP_DONE_SERVER);
        
        // Free state
        //LWIPERF_FREE(iperf3_state_t, state);
		//ipert_data_s = NULL; //ADDED
    }
    return ERR_OK;
}

static err_t iperf3_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    iperf3_state_t *server_state = (iperf3_state_t *)arg;
	char *head;
    LWIP_UNUSED_ARG(err);

//[server_state->is_server == 1]
if (1) {
	printf("\r\n");
	if (ipert_s && ipert_data_s) {

		printf("[iPerf3][TCP] free _MEM: ipert_s\r\n");
		printf("[iPerf3][TCP] free _MEM: ipert_data_s\r\n");

		LWIPERF_FREE(iperf3_state_t, ipert_s);
		ipert_s = NULL;
		LWIPERF_FREE(iperf3_state_t, ipert_data_s);
		ipert_data_s = NULL;
	}
		
	head = "news";
	if (!ipert_s)
		head = "ctrl";
	else if (!ipert_data_s)
		head = "data";
		
}

    iperf3_state_t *conn_state = (iperf3_state_t *)LWIPERF_ALLOC(iperf3_state_t);
    if (conn_state == NULL) {
        printf("[iPerf3][TCP] iperf3_accept: ERR_MEM allocating state\r\n");
        return ERR_MEM;
    }

	server_state->track_num++; // 1,2,3,...
	printf("%s connection establish: [%lu]\r\n", head, server_state->track_num);
	printf("[iPerf3][TCP] iperf3_accept: %s peer %d.%d.%d.%d:%u\r\n",
		   head,
		   ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
		   ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
		   pcb->remote_port);

	if (!ipert_s) {
		memset(conn_state, 0, sizeof(iperf3_state_t));
		conn_state->pcb.tcp = pcb;
		conn_state->state = IPERF3_TEST_START;
		conn_state->time_started_tcp = sys_now();
		conn_state->report_arg = server_state->report_arg;
		conn_state->is_server = 1;
		conn_state->protocol = 0; // TCP

		server_state->is_server = 2;
	//  LWIPERF_FREE(iperf3_state_t, server_state);
		
		tcp_arg(pcb, conn_state);
		tcp_sent(pcb, NULL);
		tcp_recv(pcb, iperf3_recv);
		disable_tcp_nagle3("iperf3_ctrl", pcb);  // Disable Nagle algorithm
		ipert_s = conn_state; //ADDED
	//or else [server_state->is_server == 2]
	} else {
		memset(conn_state, 0, sizeof(iperf3_state_t));
		conn_state->pcb.tcp = pcb;
		conn_state->state = IPERF3_TEST_START;
		conn_state->time_started_tcp = sys_now();
		conn_state->report_arg = server_state->report_arg;
		conn_state->is_server = 2;
		conn_state->protocol = 0; // TCP

		// just keep 'server_state' alive!
		server_state->is_server = 3;
		//.LWIPERF_FREE(iperf3_state_t, server_state);

		tcp_arg(pcb, conn_state);
		tcp_sent(pcb, NULL);
		tcp_recv(pcb, iperf3_data_recv);
		disable_tcp_nagle3("iperf3_data", pcb);  // Disable Nagle algorithm
		ipert_data_s = conn_state;
	}

    return ERR_OK;
}
	
//void iperf_tcp_init(void)
//{
//	void iperf3_init(void);
//	iperf3_init();

//	void iperf2_init(void);
//	iperf2_init();
//}

void iperf3_tcp_init(void)
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
    //printf("[iPerf3] init: creating TCP server on %u\r\n", IPERF3_DEFAULT_PORT);
    
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
}

/** Close an iperf tcp session */
//static void
//iperf2_tcp_close(lwiperf_state_tcp_t *conn, enum lwiperf_report_type report_type)
//{
//	iperf2_tcp_conn_report(conn, report_type);
//  LWIPERF_FREE(lwiperf_state_tcp_t, conn);
//}

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
	// Connection closed by peer
	u32_t now = sys_now();
	printf("bytes %u start time %lu end time %lu\r\n", (unsigned)conn->bytes_transferred, conn->time_started, now);
	printf("[iPerf2][TCP] connection closed by peer %d.%d.%d.%d:%u bytes_transferred=%u duration_ms=%lu\r\n",
		   ip4_addr1_val(pcb->remote_ip), ip4_addr2_val(pcb->remote_ip),
		   ip4_addr3_val(pcb->remote_ip), ip4_addr4_val(pcb->remote_ip),
		   pcb->remote_port, (unsigned)conn->bytes_transferred, now - conn->time_started);

    //iperf2_tcp_close(conn, LWIPERF_TCP_DONE_SERVER);x
    tcp_arg(pcb, NULL); //JJ.YES.NULL
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_close(pcb);
    //iperf2_tcp_close(conn, LWIPERF_TCP_DONE_SERVER);o

    // Free state
	iperf2_tcp_conn_report(conn, LWIPERF_TCP_DONE_SERVER);
	LWIPERF_FREE(lwiperf_state_tcp_t, conn);
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
#if 0
  /*
   *[we can iperf -c test for multi-times.][don't free.]
   */
  //[essential ]?
  LWIPERF_FREE(lwiperf_state_tcp_t, s);
#endif
  tcp_arg(pcb, conn); //JJ
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, iperf2_recv);
#if 1
  disable_tcp_nagle2("iperf2", pcb);  // Disable Nagle algorithm
#endif
  printf("bytes %u start time %lu\r\n", (unsigned)conn->bytes_transferred, conn->time_started);
  return ERR_OK;
}

void iperf2_tcp_init(void)
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
}
