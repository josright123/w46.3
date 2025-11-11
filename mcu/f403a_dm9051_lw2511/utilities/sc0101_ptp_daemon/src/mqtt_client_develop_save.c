

#if 1
#endif

#if 1
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    // ?????????
    printf("[recv][MQTT] <- Received message on topic: %s (length: %u bytes)\r\n", topic, tot_len);

    // ???????????????
    if (tot_len > MQTT_PAYLOAD_BUFFER_SIZE - 1) {
        printf("[MQTT] WARNING: Message payload too large (%u > %u bytes), truncating\r\n", 
               tot_len, MQTT_PAYLOAD_BUFFER_SIZE - 1);
    }
}

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    // ????????????
    if (len > 0 && len < MQTT_PAYLOAD_BUFFER_SIZE) {
        // ????????? null ??
        char buffer[MQTT_PAYLOAD_BUFFER_SIZE];
        memcpy(buffer, data, len);
        buffer[len] = '\0';

        // ???????????????
        printf("[recv][MQTT] Received data (%d bytes): %s\r\n", len, buffer);
        
        // ??????????
        if (strstr(buffer, "STATUS") != NULL) {
            printf("[MQTT] Status request received\r\n");
            // ???????????
        }
    } else {
        printf("[MQTT] Received data too large or empty (%d bytes)\r\n", len);
    }
}

// MQTT ??????
void mqtt_sub_cb(void *arg, err_t result) {
    // ??????
    if (result == ERR_OK) {
        printf("[MQTT] Subscription successful\r\n");
    } else {
        printf("[MQTT] Subscription failed with error: %d\r\n", result);
        
        // ??????
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
#endif

// ??????
void set_callbacks(mqtt_client_t *client) {
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
}

// ?????????
void subscribe_to_topics(mqtt_client_t *client, const mqtt_topic_info *topic_array, int topic_count) {
    for (int i = 0; i < topic_count; i++) {
  printf("sub [%d/%d]\r\n", i+1, topic_count);
        mqtt_subscribe(client, topic_array[i].topic, topic_array[i].qos, mqtt_sub_cb, NULL);
    }
}



#if 1
// MQTT ??????
static void mqtt_client_pub_request_cb(void *arg, err_t result) {
    static int callback_count = 0;
    callback_count++;

    // ??????
    if (result == ERR_OK) {
        //printf("[MQTT] -> [OK] Publish confirmed by broker! (callback #%d)\r\n", callback_count);
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

void publish_to_multiple_topics(mqtt_client_t *client, const mqtt_topic_info *topics, int num_topics) {
    char payload[MQTT_PAYLOAD_BUFFER_SIZE];
    static int msg_count = 0;
    static int success_count = 0;
    static int error_count = 0;
    
    // ????????(?? local_time ???,?????????)
    extern volatile uint32_t local_time;  // ? main.c ??
    uint32_t uptime_sec = local_time / 1000;  // ????
    
    // NOT (??? JSON ??? payload)
    sprintf(payload, "{ \"at32f403a_ptp_daemon\" %d}", ++pub_msg_index);
    printf("[MQTT] Publishing [%d/%d]: %s\r\n", topic_index+1, num_topics, payload);

    // ????????,???????
    int result = mqtt_publish(client, topics[topic_index].topic, payload, strlen(payload), topics[topic_index].qos, 0, mqtt_client_pub_request_cb, NULL);
    if (result != ERR_OK) {
        error_count++;
        printf("[MQTT] [FAIL] Failed to publish to topic %s, error=%d (total errors: %d)\r\n", 
               topics[topic_index].topic, result, error_count);
        
        // ??????
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

    // ??????????
    topic_index = (topic_index + 1) % num_topics;
    
    // ?10?????????
    if (msg_count % 10 == 0) {
        printf("[MQTT] [STATS] Total=%d, Success=%d, Errors=%d, Success Rate=%.1f%%\r\n", 
               msg_count, success_count, error_count, 
               (float)success_count * 100.0f / (float)msg_count);
    }
}
#endif
