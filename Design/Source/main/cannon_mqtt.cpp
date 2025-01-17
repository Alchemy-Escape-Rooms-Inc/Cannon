#include "cannon_mqtt.h"

#include "mqtt.h"
#include <algorithm>

namespace Cannon
{
    MqttTopic::MqttTopic(PayloadType type) : type(type)
    {
        
    }

    void MqttTopic::publish(int value)
    {
        std::string payload;

        switch (type)
        {
            case PayloadType::Trigger:
                payload = mqtt_triggered_payload;
            break;

            case PayloadType::Angle:
                payload = mqtt_payload_prefix + std::to_string(std::clamp(value, 0, 359));
            break;

            case PayloadType::Percent:
                payload = mqtt_payload_prefix + std::to_string(std::clamp(value, 0, 100));
            break;

            case PayloadType::Debug:
                payload = mqtt_payload_prefix + std::to_string(value);
            break;
        }

        MQTT::publish(topic, payload);
        //if (payload == mqtt_triggered_payload) MQTT::publish(topic, mqtt_init_payload);
    }

    void MqttTopic::init(uint8_t id, std::string name)
    {
        topic = mqtt_topic_base + "Cannon" + std::to_string(id) + name;
        reset();
    }

    void MqttTopic::reset()
    {
        MQTT::publish(topic, mqtt_init_payload);
    }
}
