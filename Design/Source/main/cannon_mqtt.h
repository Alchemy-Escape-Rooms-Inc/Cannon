#pragma once

#include <string>
#include <cstdint>

namespace Cannon
{
    const std::string mqtt_topic_base = "MermaidsTale/";

    const std::string mqtt_payload_prefix = "pre_";
    const std::string mqtt_triggered_payload = "triggered";
    const std::string mqtt_init_payload = "init";

    struct MqttTopic
    {
        std::string topic;

        enum class PayloadType
        {
            Trigger,
            Angle,
            Percent,
            Debug
        };

        PayloadType type;

        MqttTopic(PayloadType type);

        void init(uint8_t id, std::string name);
        void setTopic(uint8_t id, std::string name);
        void publish(int value = -1);
        void reset();
    };
}
