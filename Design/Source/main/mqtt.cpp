#include "mqtt.h"

#include <map>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define CONFIG_BROKER_URL "mqtt://192.168.0.175"

namespace MQTT
{
    namespace
    {
        std::map<std::string, SubscribeCallback> callbacks;
        esp_mqtt_client_handle_t client;

        const char *TAG = "mqtt_example";

        void log_error_if_nonzero(const char *message, int error_code)
        {
            if (error_code != 0) {
                ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
            }
        }

        void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
        {
            ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
            esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
            esp_mqtt_client_handle_t client = event->client;
            int msg_id;

            switch ((esp_mqtt_event_id_t) event_id)
            {
            case MQTT_EVENT_CONNECTED:
                ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
                msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

                msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
                ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

                msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
                ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

                msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
                ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
                break;

            case MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
                break;

            case MQTT_EVENT_SUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                break;

            case MQTT_EVENT_UNSUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
                break;

            case MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                break;

            case MQTT_EVENT_DATA:
            {
                ESP_LOGI(TAG, "MQTT_EVENT_DATA");
                printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
                printf("DATA=%.*s\r\n", event->data_len, event->data);

                if (event->data_len > 0 && event->topic_len > 0)
                {
                    std::string topic{event->topic, (size_t) event->topic_len};
                    std::string data{event->data, (size_t) event->data_len};

                    if (callbacks.contains(topic))
                    {
                        callbacks[topic](data);
                    }
                }
            }
            break;

            case MQTT_EVENT_ERROR:
                ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
                if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                    log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                    log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                    log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                    ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

                }
                break;

            case MQTT_EVENT_ANY:
                break;

            case MQTT_EVENT_BEFORE_CONNECT:
                break;

            default:
                ESP_LOGI(TAG, "Other event id:%d", event->event_id);
                break;
            }
        }

        void mqtt_app_start(void)
        {
            esp_mqtt_client_config_t mqtt_cfg = { 0 };

            mqtt_cfg.broker.address.uri = CONFIG_BROKER_URL;
            mqtt_cfg.broker.address.port = 1883;

            client = esp_mqtt_client_init(&mqtt_cfg);
            /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
            esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t) ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
            esp_mqtt_client_start(client);
        }
    }

    void init()
    {
        ESP_LOGI(TAG, "[APP] Startup..");
        ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

        esp_log_level_set("*", ESP_LOG_INFO);
        esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
        esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
        esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
        esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
        esp_log_level_set("transport", ESP_LOG_VERBOSE);
        esp_log_level_set("outbox", ESP_LOG_VERBOSE);

        // ESP_ERROR_CHECK(nvs_flash_init());
        // ESP_ERROR_CHECK(esp_netif_init());
        // ESP_ERROR_CHECK(esp_event_loop_create_default());

        mqtt_app_start();
    }

    void publish(std::string topic, std::string data)
    {
        int msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);

        if (msg_id != -1)
        {
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        }
        else
        {
            ESP_LOGI(TAG, "sent subscribe failed, msg_id=%d", msg_id);
        }
    }

    void subscribe(std::string topic, SubscribeCallback callback)
    {
        int msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);

        if (msg_id != -1)
        {
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            callbacks[topic] = callback;
        }
        else
        {
            ESP_LOGI(TAG, "sent subscribe failed, msg_id=%d", msg_id);
        }
    }
}
