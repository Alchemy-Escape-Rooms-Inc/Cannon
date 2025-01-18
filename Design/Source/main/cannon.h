#pragma once

#include "pins.h"
#include "als31300.h"
#include "VL6180X.h"
#include "fogmachine.h"

#include "cannon_mqtt.h"

namespace Cannon
{
    class Handler
    {
        static constexpr const char* TAG = "Cannon::Handler";

        MqttTopic loadedTopic{MqttTopic::PayloadType::Trigger};
        MqttTopic horTopic{MqttTopic::PayloadType::Angle};
        MqttTopic firedTopic{MqttTopic::PayloadType::Trigger};

    public:
        enum class ID : uint8_t
        {
            Left,
            Right
        };

        ID id;
        Handler(ID id) : id(id) {}

    private:    
        // Sensors and Pins
        static constexpr gpio_num_t firePin = GPIO_NUM_1;
        static constexpr gpio_num_t iotPin = GPIO_NUM_35;

        static constexpr gpio_num_t reloadIntPin = GPIO_NUM_33;
        static constexpr gpio_num_t reloadEnablePin = GPIO_NUM_34;

        ALS31300::Sensor aimSensor;
        VL6180X reloadSensor{reloadEnablePin, reloadIntPin};

        // State
        bool loaded = false;
        int64_t loadedTime = 0;
        int64_t lastAngleUpdate = 0;
        int64_t lastDepthUpdate = 0;
        int64_t lastReloadUpdate = 0;

    public:
        void init();
        void process();

    private:
        void doReload();
        void checkFire();
        void checkLoaded();
    };
}