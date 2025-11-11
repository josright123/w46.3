/**
  **************************************************************************
  * @file     mqtt_client.c
  * @version  v1.0.0
  * @date     2025-09-01
  * @brief    mqtt client config program
  **************************************************************************
  */
#include <stdio.h>
#include <string.h>
#include "lwip/ip_addr.h"
//#include "lwip/apps/mqtt.h"
#include "lwip/netif.h"
#include "netconf.h"

#include "developer_conf.h"

extern mqtt_client_t *client1;
extern volatile uint8_t mqtt_connected;

// MQTT 連接回調函數
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    printf("[MQTT] Connection callback triggered, status: %d\r\n", status);
    
    switch(status) {
        case MQTT_CONNECT_ACCEPTED:
            printf("[MQTT] Connection accepted - SUCCESS!\r\n");
            mqtt_connected = 1;
            break;
        case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
            printf("[MQTT] Connection refused - Protocol version not supported\r\n");
            mqtt_connected = 0;
            break;
        case MQTT_CONNECT_REFUSED_IDENTIFIER:
            printf("[MQTT] Connection refused - Client identifier rejected\r\n");
            mqtt_connected = 0;
            break;
        case MQTT_CONNECT_REFUSED_SERVER:
            printf("[MQTT] Connection refused - Server unavailable\r\n");
            mqtt_connected = 0;
            break;
        case MQTT_CONNECT_REFUSED_USERNAME_PASS:
            printf("[MQTT] Connection refused - Bad username or password\r\n");
            mqtt_connected = 0;
            break;
        case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
            printf("[MQTT] Connection refused - Not authorized\r\n");
            mqtt_connected = 0;
            break;
        case MQTT_CONNECT_DISCONNECTED:
            printf("[MQTT] Disconnected from broker\r\n");
            mqtt_connected = 0;
            break;
        case MQTT_CONNECT_TIMEOUT:
            printf("[MQTT] Connection timeout\r\n");
            mqtt_connected = 0;
            break;
        default:
            printf("[MQTT] Unknown connection status: %d\r\n", status);
            mqtt_connected = 0;
            break;
    }
}

// ?? MQTT ????
//#define MQTT_BROKER_IP IPADDR4_INIT_BYTES(192, 168, 249, 202)
#define MQTT_BROKER_IP IPADDR4_INIT_BYTES(192, 168, 6, 2)
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "at32f403a_ptp_client"
char mqtt_client_id[60] = {0}; // to be used to merge 'MQTT_CLIENT_ID' with mac[3]:mac[4]:mac[5]

const ip_addr_t mqttBrokerIpAddr = MQTT_BROKER_IP;

struct mqtt_connect_client_info_t ci = {
		.client_id = MQTT_CLIENT_ID,
		.client_user = NULL,
		.client_pass = NULL,
		.keep_alive = 60,         // 60 seconds keep-alive
		.will_topic = NULL,
		.will_msg = NULL,
		.will_qos = 0,
		.will_retain = 0,
#if LWIP_ALTCP && LWIP_ALTCP_TLS
		.tls_config = NULL,
#endif
};

err_t mqttc_connect(void)
{
    // ?????????
    mqtt_connected = 0;
	printf(" ------------- _mqtt_client_process_connecting _services_manager.s -------------\r\n");

    // ?? MQTT broker ??
    printf("[MQTT] MQTT Configuration:\r\n");
    printf("[MQTT] - Broker IP: %d.%d.%d.%d\r\n", 
           ip4_addr1(&mqttBrokerIpAddr), ip4_addr2(&mqttBrokerIpAddr), 
           ip4_addr3(&mqttBrokerIpAddr), ip4_addr4(&mqttBrokerIpAddr));
    printf("[MQTT] - Broker Port: %d\r\n", MQTT_BROKER_PORT);
    printf("[MQTT] - Client ID: %s\r\n", ci.client_id);
    printf("[MQTT] - Keep Alive: %d seconds\r\n", ci.keep_alive);

    // ?? MQTT ???
    printf("[MQTT] ========================================\r\n");
    printf("[MQTT] Attempting to connect to MQTT broker...\r\n");
    
    err_t err = mqtt_client_connect(client1, &mqttBrokerIpAddr, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);

    if (err != ERR_OK) {
        printf("[MQTT] ****************************************\r\n");
        printf("[MQTT] CONNECTION FAILED - Error Analysis\r\n");
        printf("[MQTT] ****************************************\r\n");
        printf("[MQTT] ERROR: mqtt_client_connect() failed with error: %d\r\n", err);

        // ???????????
        switch(err) {
            case ERR_MEM:
                printf("[MQTT] Error detail: Out of memory\r\n");
                printf("[MQTT] Suggestion: Increase heap size or reduce MQTT buffer size\r\n");
                break;
            case ERR_BUF:
                printf("[MQTT] Error detail: Buffer error\r\n");
                printf("[MQTT] Suggestion: Check MQTT buffer configuration\r\n");
                break;
            case ERR_TIMEOUT:
                printf("[MQTT] Error detail: Timeout\r\n");
                printf("[MQTT] Suggestion: Check network connectivity and broker availability\r\n");
                break;
            case ERR_RTE:
                printf("[MQTT] Error detail: Routing problem\r\n");
                printf("[MQTT] *** CRITICAL DIAGNOSIS FOR ERR_RTE ***\r\n");
                printf("[MQTT] This error typically indicates:\r\n");
                printf("[MQTT] 1. Network interface is down or misconfigured\r\n");
                printf("[MQTT] 2. No route to destination (missing gateway)\r\n");
                printf("[MQTT] 3. DHCP has not completed successfully\r\n");
                printf("[MQTT] 4. Target IP is unreachable from current network\r\n");
                printf("[MQTT] \r\n");
                printf("[MQTT] Troubleshooting steps:\r\n");
                printf("[MQTT] - Verify DHCP is in BOUND state\r\n");
                printf("[MQTT] - Check if target is in same subnet or gateway is configured\r\n");
                printf("[MQTT] - Verify physical network connection\r\n");
                printf("[MQTT] - Test with different target IP (e.g., gateway IP)\r\n");
                break;
            case ERR_INPROGRESS:
                printf("[MQTT] Error detail: Operation in progress\r\n");
                printf("[MQTT] Suggestion: Wait for previous connection attempt to complete\r\n");
                break;
            case ERR_VAL:
                printf("[MQTT] Error detail: Illegal value\r\n");
                printf("[MQTT] Suggestion: Check IP address and port configuration\r\n");
                break;
            case ERR_WOULDBLOCK:
                printf("[MQTT] Error detail: Operation would block\r\n");
                printf("[MQTT] Suggestion: This is a temporary state, retry may succeed\r\n");
                break;
            case ERR_USE:
                printf("[MQTT] Error detail: Address in use\r\n");
                printf("[MQTT] Suggestion: Local port conflict, will retry with different port\r\n");
                break;
            case ERR_ALREADY:
                printf("[MQTT] Error detail: Already connecting\r\n");
                printf("[MQTT] Suggestion: Previous connection attempt still in progress\r\n");
                break;
            case ERR_ISCONN:
                printf("[MQTT] Error detail: Already connected\r\n");
                printf("[MQTT] Suggestion: MQTT client is already connected\r\n");
                break;
            case ERR_CONN:
                printf("[MQTT] Error detail: Not connected\r\n");
                printf("[MQTT] Suggestion: Network interface or TCP connection failed\r\n");
                break;
            case ERR_IF:
                printf("[MQTT] Error detail: Low-level netif error\r\n");
                printf("[MQTT] Suggestion: Network interface hardware or driver issue\r\n");
                break;
            case ERR_ABRT:
                printf("[MQTT] Error detail: Connection aborted\r\n");
                printf("[MQTT] Suggestion: Connection was reset by remote host or network\r\n");
                break;
            case ERR_RST:
                printf("[MQTT] Error detail: Connection reset\r\n");
                printf("[MQTT] Suggestion: Remote host refused connection or network error\r\n");
                break;
            case ERR_CLSD:
                printf("[MQTT] Error detail: Connection closed\r\n");
                break;
            case ERR_ARG:
                printf("[MQTT] Error detail: Illegal argument\r\n");
                printf("[MQTT] Suggestion: Check function parameters and client configuration\r\n");
                break;
            default:
                printf("[MQTT] Error detail: Unknown error code (%d)\r\n", err);
                printf("[MQTT] Suggestion: Check lwIP documentation for error code details\r\n");
                break;
        }
        
        // ?? ERR_RTE ???????????
        if (err == ERR_RTE) {
            printf("[MQTT] \r\n");
            printf("[MQTT] *** ERR_RTE DETAILED TROUBLESHOOTING ***\r\n");
            printf("[MQTT] Based on your configuration:\r\n");
            printf("[MQTT] - Device IP: 192.168.249.117\r\n");
            printf("[MQTT] - Gateway: 192.168.249.1\r\n");
            printf("[MQTT] - Target: 192.168.249.202\r\n");
            printf("[MQTT] \r\n");
            printf("[MQTT] Recommended actions:\r\n");
            printf("[MQTT] 1. Verify MQTT broker is running at 192.168.249.202:1883\r\n");
            printf("[MQTT] 2. Test ping to gateway: 192.168.249.1\r\n");
            printf("[MQTT] 3. Test ping to target: 192.168.249.202\r\n");
            printf("[MQTT] 4. Check if DHCP has assigned correct gateway\r\n");
            printf("[MQTT] 5. Try connecting to a different target (e.g., gateway)\r\n");
            printf("[MQTT] \r\n");
            printf("[MQTT] Alternative broker configurations to try:\r\n");
            printf("[MQTT] - Local gateway: 192.168.249.1\r\n");
            printf("[MQTT] - Public test broker: test.mosquitto.org (needs DNS)\r\n");
            printf("[MQTT] ****************************************\r\n");
        }

        printf("[MQTT] ****************************************\r\n");
//        mqtt_client_free(client1);
//        client1 = NULL;
//		if (err != ERR_OK)
//		{
		printf("[MQTT] MQTT initialization failed with error: %d\r\n", err);
		printf(" ------------- _mqtt_client_process_connecting _services_manager.e -------------\r\n");
//		}
        return err;
    }

    printf("[MQTT] Connection request sent successfully\r\n");
    printf("[MQTT] Waiting for connection callback...\r\n");
    printf("[MQTT] ========================================\r\n");
//	if (err == ERR_OK)
//	{
//	printf("[MQTT] MQTT initialization request sent -- on mqtt connecting\r\n");
	printf(" ------------- _mqtt_client_process_connecting _services_manager.e -------------\r\n");
//	}
    return ERR_OK;
}

/* NOT used 
 * For manager compare
 */
void ethernetif_services_manager(void)
{
    static uint8_t power_up_ip_done = 0;

	if (dm_ptpd_mqtt_periodic_stat())
		power_up_ip_done = 1;

	if (power_up_ip_done) {
		static uint8_t last_ethernetif_state    = 0; //0;
		uint8_t current_ethernetif_state = ethernetif_is_state();

		/* Handle network state changes */
		if (current_ethernetif_state != last_ethernetif_state)
		{
			if (current_ethernetif_state)
			{
				printf(" ------------- _ethernetif_services_manager alternative connecting link up.s -------------\r\n");
				//mqtt_connecting = mqtt_client_process_connect(mqtt_connecting);
				//if (mqtt_connecting)
				//	mqtt_subscriptions_setup = 0;  /* Reset subscription flag */

				//printf("mqttc_goto_connect\r\n");
				//printf("mqttc_todo_connect\r\n");
				//mqttc_goto_connect(netif, MY_PHY_LINKUP); /* bedo-once relate to 'netif_is_link_up()' */
				//mqttc_todo_connect();
				printf(" ------------- _ethernetif_services_manager alternative connecting link up.e -------------\r\n");
			}
			else
			{
				printf("------------- _ethernetif_services_manager link alternative stopping down.s -------------\r\n");
				//mqtt_client_process_stop();

				//mqttc_goto_close();
				//mqttc_todo_close();
				printf("------------- _ethernetif_services_manager link alternative stopping down.e -------------\r\n");
			}
			last_ethernetif_state = current_ethernetif_state;
		}
	}
}

/* call back
 * @param pub_cb Callback invoked when publish starts, contain topic and total length of payload
 */
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    // 處理收到的訂閱內容
//    printf("[recv][MQTT] <- Received message on topic: %s (length: %u bytes)\r\n", topic, tot_len);

    // 確保訊息內容不會超出緩衝區大小
    if (tot_len > MQTT_SUBSCRIBE_PAYLOAD_BUFFER_SIZE - 1) {
        printf("[MQTT] WARNING: Message payload too large (%u > %u bytes), truncating\r\n", 
               tot_len, MQTT_SUBSCRIBE_PAYLOAD_BUFFER_SIZE - 1);
    }
}

#define TOPIC_MAXLEN_SUBSCRIBE						128
char incoming_sub_topic[TOPIC_MAXLEN_SUBSCRIBE+1]; // incoming subscribed, topic.
u8_t incoming_msg_qos;

char *get_incoming_subscribe_topic(void) {
	return incoming_sub_topic;
}
u8_t get_incoming_msg_qos(void) {
	return incoming_msg_qos;
}
void set_incoming_subscribe_topic(char *str, u16_t topic_len, u8_t qos) {
	//.printf("-dbg-Receive a Subscribe message, The topic length is %u\r\n", topic_len);
	if (topic_len > TOPIC_MAXLEN_SUBSCRIBE)
		printf("warn-Receive a Subscribe message, But topic length is too large length !!!\r\n");

	memset(incoming_sub_topic, 0, sizeof(incoming_sub_topic));
	memcpy(incoming_sub_topic, str, sizeof(incoming_sub_topic) - 1);
	incoming_msg_qos = qos;
}

int size_sub_topics(void)
{
	return sizeof(aiot_sub_topic) / sizeof(aiot_sub_topic[0]);
}

/* call back
 * @param data_cb Callback for each fragment of payload that arrives
 */
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    // 處理接收到訊息數據的邏輯
    if (len > 0 && len < MQTT_SUBSCRIBE_PAYLOAD_BUFFER_SIZE) {
        // 確保接收到的數據以 null 結尾
        char buffer[MQTT_SUBSCRIBE_PAYLOAD_BUFFER_SIZE];
        memcpy(buffer, data, len);
        buffer[len] = '\0';
#if 1
		//[extra]
		  char *top = get_incoming_subscribe_topic();
#endif

        // 將接收到的數據以字符串格式顯示
  printf("%s \r\n", PTPD_HEADER_MCU_DESC);
  printf("%s [recv][MQTT] on topic: %s qos %u, Received data (%d bytes): \r\n%s\r\n", PTPD_HEADER_MCU_DESC, top, get_incoming_msg_qos(), len, buffer);
  printf("%s \r\n", PTPD_HEADER_MCU_DESC);
        
        // 檢查是否收到特殊命令
        if (strstr(buffer, "STATUS") != NULL) {
            printf("[MQTT] Status request received\r\n");
            // 可以在這裡回應狀態訊息
        }
        if (strstr(buffer, "device") != NULL) {
            printf("[MQTT] 'device' request received\r\n");
			//回應狀態訊息
			struct netif *netif = netif_default;
			printf("[MQTT] - IP: %d.%d.%d.%d\r\n", 
				   ip4_addr1(netif_ip4_addr(netif)), ip4_addr2(netif_ip4_addr(netif)),
				   ip4_addr3(netif_ip4_addr(netif)), ip4_addr4(netif_ip4_addr(netif)));
		}

		#if 0
		//[extra]
		  int i;
		  for (i = 0; i < SUB_TOPIC_QUANTITY; i++) {
			if (!strcmp(top, aiot_sub_topic[i].topic)){ //get_aiot_subscribe_topic(i)
		//		subtopic_index = i;
				printf("topic: %s, match subscribe topic %d, match!\r\n", top, i);
				break;
			}
		  }
		#endif
    } else {
        printf("[MQTT] Received data too large or empty (%d bytes)\r\n", len);
    }
}

/* call back
 * @param arg User supplied argument to subscribe callback
 * @MQTT 訂閱回調函數
 */
void mqtt_sub_cb(void *arg, err_t result) {
    // 處理訂閱結果
    if (result == ERR_OK) {
        printf("[MQTT] Subscription successful\r\n");
    } else {
        printf("[MQTT] Subscription failed with error: %d\r\n", result);
        
        // 詳細錯誤解釋
        switch(result) {
            case ERR_MEM:
                printf("[MQTT] Subscription error: Out of memory\r\n");
                break;
            case ERR_CONN:
                printf("[MQTT] Subscription error: Not connected\r\n");
                break;
            case ERR_ARG:
                printf("[MQTT] Subscription error: Invalid argument\r\n");
                break;
            default:
                printf("[MQTT] Subscription error: Unknown error\r\n");
                break;
        }
    }
}

/* call back
 * @param cb Callback to call when publish is complete or has timed out
 * @MQTT 發布回調函數
 */
static int callback_count = 0;
int get_callback_count(void)
{
	return callback_count*100;
}

void mqtt_pub_cb(void *arg, err_t result) {
	//char *topic = arg;
    callback_count++;

    // 處理發布結果
    if (result == ERR_OK) {
//        printf("[MQTT] -> [OK] Publish confirmed by broker! (callback #%d) Success Rate=%.1f%%\r\n",
//			callback_count,
//			((float)get_callback_count() / (float)get_msg_count()));
    } else {
        printf("[MQTT] -> [FAIL] Publish rejected by broker, error=%d (callback #%d)\r\n", result, callback_count);
        
        switch(result) {
            case ERR_MEM:
                printf("[MQTT] Broker callback error: Out of memory\r\n");
                break;
            case ERR_CONN:
                printf("[MQTT] Broker callback error: Connection lost\r\n");
                break;
            case ERR_ARG:
                printf("[MQTT] Broker callback error: Invalid argument\r\n");
                break;
            case ERR_TIMEOUT:
                printf("[MQTT] Broker callback error: Timeout\r\n");
                break;
            default:
                printf("[MQTT] Broker callback error: Unknown error\r\n");
                break;
        }
    }
}
