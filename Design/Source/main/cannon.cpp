#include "cannon.h"

#include "ethernet.h"
#include "mqtt.h"
#include "time.h"
#include "i2c.h"

#include "esp_log.h"

namespace Cannon
{
    void Handler::init()
    {
        if (!Ethernet::init()) esp_restart();
        if (!MQTT::init()) esp_restart();

        uint8_t topicId = id == ID::Left ? 1 : 2;
        loadedTopic.init(topicId, "Loaded");
        horTopic.init(topicId, "Hor");
        firedTopic.init(topicId, "Fired");

        Pins::initPin(firePin, GPIO_MODE_INPUT);
        Pins::initPin(iotPin, GPIO_MODE_OUTPUT, true);

        I2C::init();
        aimSensor.init(1);
        reloadSensor.init();
        fogMachine.init();

        esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    }

    void Handler::process()
    {
        if (!reloadSensor.isInitialized()) reloadSensor.init();
        else
        {
            if (reloadSensor.triggered()) doReload();
        }

        checkLoaded();

        // if (Time::elapsed(lastDepthUpdate, 50))
        // {
        //     lastDepthUpdate = Time::ms();

        //     uint8_t status = reloadSensor.readRangeStatus();
        //     VL6180X::ReadStatus err = reloadSensor.readRange(range);

        //     if (err == VL6180X::ReadStatus::ReadOkay && status == VL6180X_ERROR_NONE)
        //     {
        //         
        //         depthDebugTopic.publish(range);
        //     }
        //     else
        //     {
        //         ESP_LOGE("Cannon", "Status: %u, read result: %u", status, (int) err);
        //     }
        // }
        
        if (aimSensor.update())
        {
            if (Time::elapsed(lastAngleUpdate, 100))
            {
                lastAngleUpdate = Time::ms();
                horTopic.publish(aimSensor.getAngle());
            }
        }

        if (Pins::getInput(firePin))
        {
            checkFire();
        }

        fogMachine.process();
    }

    void Handler::doReload()
    {
        ESP_LOGI(TAG, "Reload Pulse!");

        if (loaded) return;
        if (!loadedTime) loadedTime = Time::ms();
    }

    void Handler::checkLoaded()
    {
        if (loadedTime && Time::elapsed(loadedTime, 500))
        {
            ESP_LOGI(TAG, "Reload Triggered!");
            loadedTime = 0;
            loaded = true;
            loadedTopic.publish();
            firedTopic.reset();
        }
    }

    void Handler::checkFire()
    {
        if (loaded)
        {
            ESP_LOGI(TAG, "Fire Triggered!");
            loaded = false;
            loadedTime = 0;
            firedTopic.publish();
            loadedTopic.reset();
            fogMachine.trigger();
        }
    }
}
