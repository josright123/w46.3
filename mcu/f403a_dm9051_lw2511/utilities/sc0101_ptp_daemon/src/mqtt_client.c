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

#define DEVELOPER_CONF
#include "developer_conf.h"

mqtt_client_t *client1 = NULL;

// MQTT 連接狀態標誌 (替換 FreeRTOS 信號量)
//static 
volatile uint8_t mqtt_connected = 0;
int topic_index = 0;  // 輪流發布單一主題
int pub_msg_index = 0;

int mqttproc_check_connection(void)
{
    return mqtt_connected;
}

/** @addtogroup UTILITIES_examples
 * @{
 */

/** @addtogroup FreeRTOS_demo
 * @{
 */

#if 0
//err_t mqtt_client_process_init(void)
//{
//}
//uint8_t mqtt_client_process_connect(uint8_t connecting) //xx_network_services_manager
//{
////    static uint8_t last_network_state    = 0;
////    uint8_t        current_network_state = network_dhcp_is_bound();
////    /* Handle network state changes */
////    if (current_network_state != last_network_state)
////    {
////        if (current_network_state)
////        {
////          printf("[NETWORK] Network connected, starting services...\r\n");

//			/* Initialize MQTT when network is already mqtt init */
//			if (connecting)
//				return 1;

//			ethernetif_info();
//			if (mqttc_connect() == ERR_OK)
//				return 1;

//			return 0;

////        }
////        else
////        {
////            /* Network disconnected, stop services */
////            printf("[NETWORK] Network disconnected, stopping services...\r\n");
////            
////            /* Reset MQTT state when network is disconnected */
////            mqtt_initialized = 0;
////            mqtt_subscriptions_setup = 0;
////            printf("[MQTT] MQTT services stopped due to network disconnection\r\n");
////        }
////        last_network_state = current_network_state;
////    }
//}
#endif

//=mqttc_init();
err_t mqtt_client_process_init(void)
{
  printf("[MQTT] mqttc_init(); Starting MQTT initialization...\r\n"); //=mqttc_init();

  if (client1 == NULL)
    client1 = mqtt_client_new();
  if (client1 == NULL) {
	printf("[MQTT] ERROR: Failed to create MQTT client\r\n");
	return ERR_MEM;
  }
  printf("[MQTT] MQTT client created successfully\r\n");
  return ERR_OK;
}

uint8_t mqtt_client_process_connect(uint8_t connecting) //xx_network_services_manager
{
	/* Initialize MQTT when network is already mqtt init */
	if (connecting)
		return 1;

	ethernetif_info();
	if (mqttc_connect() == ERR_OK)
		return 1;

	return 0;
}

void mqtt_client_process_stop(void) //xx_network_services_manager
{
}

void mqttproc_setup_subscriptions(void)
{
    // 訂閱主題
    subscribe_to_topics(client1, aiot_sub_topic, SUB_TOPIC_QUANTITY); // size_pub_topics_sub_topics()
}

// 設置回調函數
void set_callbacks(void) {
    mqtt_set_inpub_callback(client1, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
}

// 訂閱所有主題的函數
void subscribe_to_topics(mqtt_client_t *client, const mqtt_topic_info *topic_array, int topic_count) {
    for (int i = 0; i < topic_count; i++) {
  printf(".%d ------------------------------------------------------------\r\n", i);
  printf(" Subscribe topic %s, qos %u [%d/%d]\r\n", topic_array[i].topic, topic_array[i].qos, i+1, topic_count);
        mqtt_subscribe(client, topic_array[i].topic, topic_array[i].qos, mqtt_sub_cb, NULL);
    }
}

static int msg_count = 0;
int get_msg_count(void)
{
	return msg_count;
}

int mqttproc_publish(uint32_t localtime, uint32_t last_publish_time)
{
	printf("\r\n");
	printf(".%d ------------------------------------------------------------\r\n", (localtime - last_publish_time));
	printf("[%d/%d] Publishing...\r\n",
		topic_index + 1,
		size_pub_topics_sub_topics()
		);

    // 發佈訊息
	msg_count++;
    publish_to_multiple_topics(client1, localtime - last_publish_time);
	return msg_count;
}

#define MQTT_PUB_PAYLOAD_BUFFER_SIZE					256

void publish_to_multiple_topics(mqtt_client_t *client, uint32_t esc_time) {
    static int success_count = 0;
    static int error_count = 0;
    char payload[MQTT_PUB_PAYLOAD_BUFFER_SIZE];
    
    // 取得系統運行時間（假設 local_time 可存取，否則使用靜態計數器）
    //extern volatile uint32_t local_time;  // 從 main.c 引用

	sprintf(payload, "{ \"at32f403a_ptp_daemon\": %d, \"up time\": %lu s}", ++pub_msg_index, esc_time / 1000); // 轉換為秒

    // 簡化的 JSON 格式的 payload 
//#if 0
//    - {
//		 \"device\":\"AT32F403A_PTP\",
//		 \"timestamp\":%u,
//		 \"msg_id\":%d,
//		 \"mqtt\":{
//			\"total\":%d,
//			\"success\":%d,
//			\"errors\":%d,
//			\"topic_idx\":%d
//		  },
//		 \"system\":{
//			\"uptime_sec\":%u,
//			\"mem_size\":%d
//		  }
//		}
//    sprintf(payload, "{\"device\":\"AT32F403A_PTP\",\"timestamp\":%u,\"msg_id\":%d,\"mqtt\":{\"total\":%d,\"success\":%d,\"errors\":%d,\"topic_idx\":%d},\"system\":{\"uptime_sec\":%u,\"mem_size\":%d}}", 
//            local_time, msg_count, msg_count-1, success_count, error_count, topic_index, uptime_sec, MEM_SIZE);
//#else
//#endif

    int num_topics = size_pub_topics_sub_topics();

	const mqtt_topic_info *topics = aiot_pub_topic;
    printf("[MQTT] Publishing topic %s, qos %u [%d/%d]: %s\r\n", topics[topic_index].topic, topics[topic_index].qos, topic_index+1, num_topics, payload);

    // 只發布到一個主題，避免記憶體問題
    int result = mqtt_publish(client, topics[topic_index].topic, payload, strlen(payload), topics[topic_index].qos, 0, mqtt_pub_cb, NULL);
    if (result != ERR_OK) {
        error_count++;
        printf("[MQTT] [FAIL] Failed to publish to topic %s, error=%d (total errors: %d)\r\n", 
               topics[topic_index].topic, result, error_count);
        
        // 詳細錯誤解釋
        switch(result) {
            case ERR_MEM:
                printf("[MQTT] Publish error: Out of memory\r\n");
                break;
            case ERR_BUF:
                printf("[MQTT] Publish error: Buffer error\r\n");
                break;
            case ERR_CONN:
                printf("[MQTT] Publish error: Not connected\r\n");
                break;
            case ERR_ARG:
                printf("[MQTT] Publish error: Invalid argument\r\n");
                break;
            default:
                printf("[MQTT] Publish error: Unknown error\r\n");
                break;
        }
    } else {
        success_count++;
        printf("[MQTT] [OK] Successfully queued for topic: %s (total success: %d)\r\n", 
               topics[topic_index].topic, success_count);
    }

    // 輪流切換到下一個主題
    topic_index = (topic_index + 1) % num_topics;
}

static uint8_t mqtt_connecting = 0; //<========= as <========== mqtt_initialized = 0;
static uint8_t mqtt_subscriptions_setup = 0;
static uint32_t mqtt_last_publish_time = 0;
#define MQTT_PUBLISH_INTERVAL_MS 50000  /* 每50秒發布一次訊息 */

void mqtt_connecting_process(void)
{
	mqtt_connecting = mqtt_client_process_connect(mqtt_connecting);
	if (mqtt_connecting)
		mqtt_subscriptions_setup = 0;  /* Reset subscription flag */
}

void dm_mqtt_periodic(volatile uint32_t localtime)
{
	/* MQTT services management */
	static uint32_t last_mqtt_debug_time = 0;
	static uint8_t last_mqtt_connected = 0;

	int timer_expired = 0, connected_chg = 0;

	if ((localtime - last_mqtt_debug_time) >= 5000) {
		last_mqtt_debug_time = localtime;
		timer_expired = 1;
	}
	uint8_t current_mqtt_connected = mqttproc_check_connection();
	if (last_mqtt_connected != current_mqtt_connected) {
		last_mqtt_debug_time = localtime;
		last_mqtt_connected = current_mqtt_connected;
		connected_chg = 1;
	}

	if (timer_expired) {
		/* Debug 訊息：每5秒顯示未mqtt initialize */
		if (!mqtt_connecting)
			printf("[MQTT] Not connected yet\r\n");
	}

	if (timer_expired || connected_chg) {
			/* Debug 訊息：每5秒或連線狀態改變時顯示 */
			static int connecting_debug_counter = 0;
			if (mqtt_connecting &&
				(/*!mqtt_connecting ||*/ !current_mqtt_connected || !mqtt_subscriptions_setup)) {
				if (connecting_debug_counter < 3) {
					connecting_debug_counter++;
					if (connecting_debug_counter == 3)
						printf("[DONE] Status - Initialized: %s, Connected: %s, Subscribed: %s\r\n",
						   mqtt_connecting ? "YES" : "NO",
						   current_mqtt_connected ? "YES" : "NO",
						   mqtt_subscriptions_setup ? "YES" : "NO");
					else
						printf("[MQTT] Status - Initialized: %s, Connected: %s, Subscribed: %s\r\n",
						   mqtt_connecting ? "YES" : "NO",
						   current_mqtt_connected ? "YES" : "NO",
						   mqtt_subscriptions_setup ? "YES" : "NO");
				}
		}
		else
			connecting_debug_counter = 0;
	}

	publish_timer(NULL, localtime);
}

//static 
//mqtt_client_t *client, 
void publish_topic(const char *topic, int qos, char *payload)
{
	if (mqtt_connecting)
	{
		if (mqttproc_check_connection())
		{
				if (mqttproc_check_connection())
				{

					// 只發布到一個主題，避免記憶體問題
					int result = mqtt_publish(client1, topic, payload, strlen(payload), qos, 0, mqtt_pub_cb, /*NULL*/(char *)topic);
					if (result != ERR_OK) {
						// 詳細錯誤解釋
						switch(result) {
							case ERR_MEM:
								printf("[MQTT] Publish error: Out of memory\r\n");
								break;
							case ERR_BUF:
								printf("[MQTT] Publish error: Buffer error\r\n");
								break;
							case ERR_CONN:
								printf("[MQTT] Publish error: Not connected\r\n");
								break;
							case ERR_ARG:
								printf("[MQTT] Publish error: Invalid argument\r\n");
								break;
							default:
								printf("[MQTT] Publish error: Unknown error\r\n");
								break;
						}
					} 
					#if 0
					else {
						printf("[MQTT] [OK] Successfully queued for topic: %s\r\n", 
							   topic);
					}
					#endif
				}
		}
	}
}

void publish_timer(void *arg, volatile uint32_t localtime)
{
	static int sent_msg_count = 0;
	if (mqtt_connecting)
	{
		if (mqttproc_check_connection())
		{

			/* Setup subscriptions once after connection is established */
			if (!mqtt_subscriptions_setup)
			{
				printf("[MQTT] Listing publishments...\r\n");
				for (int i = 0; i < size_pub_topics_sub_topics(); i++) {
				  printf(" Publishment topic %s, qos %u [%d/%d]\r\n", aiot_pub_topic[i].topic, aiot_pub_topic[i].qos, i+1, size_pub_topics_sub_topics());
				}
				printf("[PTPD] Listing publishments...\r\n");
				for (int i = 0; i < size_pub_topics_of_ptpd(); i++) {
				  printf(" Publishment topic %s, qos %u [%d/%d]\r\n", ptpd_pub_topic[i].topic, ptpd_pub_topic[i].qos, i+1, size_pub_topics_of_ptpd());
				}

				printf("[MQTT] Setting up subscriptions...\r\n");

				// 設定回調函數
				set_callbacks();

				mqttproc_setup_subscriptions();
				mqtt_subscriptions_setup = 1;
				mqtt_last_publish_time = localtime;
//				printf("[MQTT] Subscriptions setup completed\r\n");
			}
			
			if (sent_msg_count < 3)
			{
				static uint32_t iter_time = MQTT_PUBLISH_INTERVAL_MS/5;
	#if 1
				/* Periodic message publishing - debug */
				if ((localtime - mqtt_last_publish_time) > iter_time)
				{
					iter_time += MQTT_PUBLISH_INTERVAL_MS/5;
					printf(".%d ...\r\n", (localtime - mqtt_last_publish_time));
				}
	#endif
				/* Periodic message publishing */
				if ((localtime - mqtt_last_publish_time) >= MQTT_PUBLISH_INTERVAL_MS)
				{
					if (mqttproc_check_connection())
					{
						sent_msg_count = mqttproc_publish(localtime, mqtt_last_publish_time);
					}
					else
					{
						printf("[MQTT] Publish failed with error: %d\r\n", ERR_CONN);
					}

					iter_time = MQTT_PUBLISH_INTERVAL_MS/5;
					mqtt_last_publish_time = localtime;
				}
			}
		}
	}
}

void publish_sync_info(char *buf)
{
	publish_topic(ptpd_pub_topic[SYNCMSG_INDEX].topic, ptpd_pub_topic[SYNCMSG_INDEX].qos, buf); //"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/smsg"
}

//#if 1
//#include "developer_conf.h"
//#endif

/**
 * @}
 */

/**
 * @}
 */
