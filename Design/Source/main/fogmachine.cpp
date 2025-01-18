#include "fogmachine.h"

#include "pinout.h"
#include "time.h"

#include <esp_log.h>

namespace FogMachine
{
    namespace
    {
        constexpr size_t ON_TIME = 1000;
        constexpr size_t OFF_INTERVAL = 5000;

        constexpr const char* TAG = "FogMachine";
        constexpr dmx_port_t dmx_num = DMX_NUM_2;

        bool initialized = false;

        bool doTrigger = false;
        int64_t triggerTime = 0;
        int64_t offTime = 0;

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

        void sendDmxRequest(bool enable);
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
            }

            if (triggerTime)
            {
                if (!Time::elapsed(triggerTime, ON_TIME))
                {
                    // Repeatedly send commands in case they fail
                    sendDmxRequest(true);
                }
                else
                {
                    // Stop Sending
                    ESP_LOGE(TAG, "Stopping fog");
                    triggerTime = 0;
                    offTime = Time::ms();
                    sendDmxRequest(false);
                }
            }
            else if (Time::elapsed(offTime, OFF_INTERVAL))
            {
                offTime = Time::ms();
                sendDmxRequest(false); // Send periodic stop in case it gets missed
            }

            doTrigger = false;
        }

        void sendDmxRequest(bool enable)
        {
            constexpr size_t bufferSize = 16;
            uint8_t data[bufferSize] = {0};

            data[0] = 0x0;                   // Start code needs to be zero
            data[1] = enable ? 0xFF : 0x0;   // Fog level 0-255
            //data[2] = enable ? 0xFF : 0x0; // Red LED on this model I'm testing with, https://www.chauvetdj.com/wp-content/uploads/2017/10/Geyser_P7_UM_Rev2_WO.pdf

            size_t sent = 0;
            
            sent = dmx_write(dmx_num, &data, bufferSize);
            if (sent != bufferSize) ESP_LOGE(TAG, "Wrong number of bytes written to driver!");
            
            sent = dmx_send_num(dmx_num, bufferSize);
            if (sent == 0) ESP_LOGE(TAG, "No bytes sent!");

            if (!dmx_wait_sent(dmx_num, DMX_TIMEOUT_TICK)) ESP_LOGE(TAG, "Timeout sending, failed!");
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
        // Called from separate thread
        doTrigger = true;
    }
}
