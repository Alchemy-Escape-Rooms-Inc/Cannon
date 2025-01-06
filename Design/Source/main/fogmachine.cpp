#include "fogmachine.h"

#include "pinout.h"
#include "time.h"

void FogMachine::init()
{
    dmx_config_t config = DMX_CONFIG_DEFAULT;

    dmx_driver_install(dmx_num, &config, nullptr, 0);
    dmx_set_pin(dmx_num, DMX_TX_PIN, DMX_RX_PIN, DMX_EN_PIN);
}

void FogMachine::trigger()
{
    triggerTime = Time::ms();

    sendDmxRequest(true);
}

void FogMachine::process()
{
    if (triggerTime && Time::elapsed(triggerTime, 1000))
    {
        triggerTime = 0;
        sendDmxRequest(false);
    }
}

void FogMachine::sendDmxRequest(bool enable)
{
    uint8_t data = enable ? 0xFF : 0x0;

    dmx_send_num(dmx_num, 1);
    dmx_wait_sent(dmx_num, DMX_TIMEOUT_TICK);
    dmx_write_offset(dmx_num, 0, &data, 1);
}
