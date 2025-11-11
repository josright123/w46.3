#ifndef __DEVELOP_CNF_H
#define __DEVELOP_CNF_H
#include "lwip/apps/mqtt.h"

#define APPLICATION_NAME							"Davicom mqtt_client"
#define APPLICATION_BANNER							"AT32F4_DM9051_Lw212_mqtt_r2307_rc1.3-Ted"
#define APPLICATION_DATE							"2023-08-09-Ted"
#define PTPD_HEADER_MCU_DESC						"f403a"

// 宣告一個 struct 來存儲主題和 QoS
typedef struct {
    const char *topic;
    int qos;
} mqtt_topic_info;

// #define MQTT_TOPIC "example_topic"
// #define MQTT_TOPIC1 "example_topic/testTopic"
// #define MQTT_TOPIC2 "example_topic/test_Topic2"

//const mqtt_topic_info ___topics_00001[] = {
//    {MQTT_TOPIC, 0},
//    {MQTT_TOPIC1, 0},
//    {MQTT_TOPIC2, 0}
//    // 暫時註解掉其他主題以避免記憶體不足
//    // {MQTT_TOPIC3, 0},
//    // {MQTT_TOPIC4, 0},
//    // {MQTT_TOPIC5, 0},
//};

#define MQTT_MAX_PUBLISH_TOPIC_NUM					12 //use to allocate space for operation for 'MQTT_CLIENT_PUBLISH_TOPIC_NUM' 
#define MQTT_MAX_SUBSCRIBE_TOPIC_NUM				12 //only const. (when operation, use 'MQTT_CLIENT_SUBSCRIBE_TOPIC_NUM')

//-------- No, const mqtt_topic_info aiot_pub_topic[MQTT_MAX_PUBLISH_TOPIC_NUM];
//------- Yes, const mqtt_topic_info aiot_pub_topic[];
static const mqtt_topic_info aiot_pub_topic[] = {
	/*
	 * Note: We add a response design, which could occipy 1 publish topic,
	 * Do _MQTT_CLIENT_PUBLISH_TOPIC_NUM for specific publish topic number (0 ~ (_MQTT_CLIENT_PUBLISH_TOPIC_NUM-1))
	 * and 1 topic as equal to index as _MQTT_CLIENT_PUBLISH_TOPIC_NUM
	 *
	 * So make (0 ~ _MQTT_MAX_PUBLISH_TOPIC_NUM - 1) and remainder 1 for as 'DeviceTopicR1_Ack'
	 * So 
	 * it is 0 ~ (_MQTT_CLIENT_PUBLISH_TOPIC_NUM -1),
	 * and _MQTT_CLIENT_PUBLISH_TOPIC_NUM ('DeviceTopicR1_Ack') as extra 1.
	 */

//	{"/a1fwVnPM8wU/dm9051_mqttbks/user/DeviceTopicR0", 0},
//	{"/a1fwVnPM8wU/dm9051_mqttbks/user/DeviceTopicR1_Ack", 0},

//	{"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/slave", 0},
//	{"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/slave", 0},
//	{"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/slave", 0},
	
	{"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/slave", 0},
};

#define SYNCMSG_INDEX	0
#define OFFSET_INDEX	1
static const mqtt_topic_info ptpd_pub_topic[] = {
	{"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/smsg", 0},
	{"/a1fwVnPM8wU/dm9051_mqttbks/f403a/ptpdaemon/offset", 0},
};

/*
 * static const mqtt_topic_info subtopics[MQTT_MAX_SUBSCRIBE_TOPIC_NUM] = { \
 *	{"/a1fwVnPM8wU/dm9051_mqttbks/user/ResponseTopicR0", 0}, \
 *	{"/a1fwVnPM8wU/dm9051_mqttbks/user/ResponseTopicR1_bkw", 0}, \
 *	{"/a1fwVnPM8wU/dm9051_mqttbks/user/ResponseTopicR2_piston", 0}, \
 * };
 */

#ifdef DEVELOPER_CONF
static int size_pub_topics_sub_topics(void)
{
	return sizeof(aiot_pub_topic) / sizeof(aiot_pub_topic[0]);
}
static int size_pub_topics_of_ptpd(void)
{
	return sizeof(ptpd_pub_topic) / sizeof(ptpd_pub_topic[0]);
}
#endif //DEVELOPER_CONF

extern const mqtt_topic_info aiot_pub_topic[];
#define aiot_sub_topic aiot_pub_topic
int size_sub_topics(void);
#define SUB_TOPIC_QUANTITY	1

#define bannerline()								printf("\r\n")

/* =#include "main.h"								//included by "developer_conf.h"
 * (0/1) implemented in "mqtt.c" */
#define MQTT_C_DEBUG								0

int get_msg_count(void);
int size_pub_topics_sub_topics(void);

void ethernetif_info(void);
uint8_t ethernetif_is_state(void);
int mqttproc_check_connection(void);
err_t mqttc_connect(void); //int mqttproc_connect(void);
void mqtt_connecting_process(void);
void ethernetif_services_manager(void);
void dm_mqtt_periodic(volatile uint32_t localtime);
void publish_timer(void *arg, volatile uint32_t localtime);

// 設置回調函數
void set_callbacks(void);
// 訂閱所有主題的函數
void subscribe_to_topics(mqtt_client_t *client, const mqtt_topic_info *topic_array, int topic_count);
// 發佈所有主題的函數
void publish_to_multiple_topics(mqtt_client_t *client, uint32_t esc_time);

err_t mqtt_client_process_init(void);
uint8_t mqtt_client_process_connect(uint8_t connecting);
void mqtt_client_process_stop(void);
void mqttproc_setup_subscriptions(void);
int mqttproc_publish(uint32_t localtime, uint32_t last_publish_time);

// MQTT 連接回調函數
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
// 處理收到的訂閱內容
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
// 處理接收到訊息數據的邏輯
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
// MQTT 訂閱回調函數
void mqtt_sub_cb(void *arg, err_t result);
// MQTT 發布回調函數
void mqtt_pub_cb(void *arg, err_t result);

#endif //__DEVELOP_CNF_H
