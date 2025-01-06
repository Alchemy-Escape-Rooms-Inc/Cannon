#include "cannon_mqtt.h"

#include "mqtt.h"
#include <algorithm>

namespace Cannon
{
    MqttTopic::MqttTopic(PayloadType type) : type(type)
    {
        
    }

    void MqttTopic::setTopic(uint8_t id, std::string name)
    {
        topic = mqtt_topic_base + "Cannon" + std::to_string(id) + name;
    }

    void MqttTopic::publish(int value)
    {
        std::string payload;

        switch (type)
        {
            case PayloadType::Trigger:
                payload = "triggered";
            break;

            case PayloadType::Angle:
                payload = mqtt_payload_prefix + std::to_string(std::clamp(value, 0, 359));
            break;

            case PayloadType::Percent:
                payload = mqtt_payload_prefix + std::to_string(std::clamp(value, 0, 100));
            break;
        }

        MQTT::publish(topic, payload);
    }

    void MqttTopic::init()
    {
        MQTT::publish(topic, mqtt_init_payload);
    }
}
