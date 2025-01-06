#pragma once

#include <string>

namespace MQTT
{
    typedef void (*SubscribeCallback)(std::string data);

    void init();
    void publish(std::string topic, std::string payload);
    void subscribe(std::string topic, SubscribeCallback callback);
}