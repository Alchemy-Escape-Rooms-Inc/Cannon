#pragma once

#include "pins.h"
#include "als31300.h"
#include "vl53l0x.h"
#include "fogmachine.h"

#include "cannon_mqtt.h"

namespace Cannon
{
    class Handler
    {
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
        static constexpr gpio_num_t firePin = GPIO_NUM_4;
        static constexpr gpio_num_t iotPin = GPIO_NUM_5;

        ALS31300::Sensor aimSensor;
        VL53L0X reloadSensor;
        FogMachine fogMachine;

        // State
        bool loaded = false;
        int64_t loadedTime = 0;
        int64_t lastAngleUpdate = 0;

    public:
        void init();
        void process();

    private:
        void checkFire();
        void checkLoaded(uint16_t distance);
    };
}