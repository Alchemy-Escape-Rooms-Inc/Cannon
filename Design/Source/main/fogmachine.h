#pragma once

#include "esp_dmx.h"

class FogMachine
{
public:
    void init();
    void trigger();
    void process();
    int64_t triggerTime = 0;

private:
    void sendDmxRequest(bool enable);

    static constexpr dmx_port_t dmx_num = DMX_NUM_1;
};
