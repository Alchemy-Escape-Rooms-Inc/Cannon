#include "fogmachine.h"

#include "pinout.h"
#include "time.h"

#include <esp_log.h>

namespace FogMachine
{
    namespace
    {
        constexpr const char* TAG = "FogMachine";
        constexpr dmx_port_t dmx_num = DMX_NUM_2;

        bool initialized = false;
        bool doTrigger = false;
        int64_t triggerTime = 0;

        void sendDmxRequest(bool enable)
        {
            uint8_t data = enable ? 0xFF : 0x0;

            volatile size_t sent = 0;
            
            sent = dmx_write_offset(dmx_num, 0, &data, 1);
            if (sent != 1) ESP_LOGE(TAG, "Wrong number of bytes written to driver!");
            
            sent = dmx_send_num(dmx_num, 1);
            if (sent == 0) ESP_LOGE(TAG, "No bytes sent!");

            if (!dmx_wait_sent(dmx_num, DMX_TIMEOUT_TICK)) ESP_LOGE(TAG, "Timeout sending, failed!");
        }

        void init()
        {
            Pins::resetPin((gpio_num_t) DMX_TX_PIN);
            Pins::resetPin((gpio_num_t) DMX_RX_PIN);
            Pins::resetPin((gpio_num_t) DMX_EN_PIN);

            dmx_config_t config = DMX_CONFIG_DEFAULT;

            if (!dmx_driver_install(dmx_num, &config, nullptr, 0)) return;
            if (!dmx_set_pin(dmx_num, DMX_TX_PIN, DMX_RX_PIN, DMX_EN_PIN)) return;

            esp_log_level_set(TAG, ESP_LOG_VERBOSE); // Not working for some reason, using ESP_LOGE
            ESP_LOGE(TAG, "Init success");
            initialized = true;
        }

        void process()
        {
            if (!initialized)
            {
                init();
                return;
            }

            if (doTrigger && !triggerTime)
            {
                ESP_LOGE(TAG, "Starting fog");
                triggerTime = Time::ms();
                sendDmxRequest(true);
            }

            if (triggerTime && Time::elapsed(triggerTime, 1000))
            {
                ESP_LOGE(TAG, "Stopping fog");
                triggerTime = 0;
                sendDmxRequest(false);
            }

            doTrigger = false;
        }
    }

    void task(void*)
    {
        init();

        while (1)
        {
            process();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    void trigger()
    {
        doTrigger = true;
    }
}
