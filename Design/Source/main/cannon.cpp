#include "cannon.h"

#include "ethernet.h"
#include "mqtt.h"
#include "time.h"

namespace Cannon
{
    void Handler::init()
    {
        Ethernet::init();
        MQTT::init();

        uint8_t topicId = id == ID::Left ? 1 : 2;
        loadedTopic.setTopic(topicId, "Loaded");
        horTopic.setTopic(topicId, "Hor");
        firedTopic.setTopic(topicId, "Fired");

        loadedTopic.init();
        horTopic.init();
        firedTopic.init();

        Pins::initPin(firePin, GPIO_MODE_INPUT);

        //aimSensor.init(1);
        //reloadSensor.begin();
        fogMachine.init();
    }

    void Handler::process()
    {
        VL53L0X_RangingMeasurementData_t measure;
        //reloadSensor.rangingTest(&measure);

        // if (measure.RangeStatus == 0) // != 4)
        // { 
        //     checkLoaded(measure.RangeMilliMeter);
        // }
        
        // if (aimSensor.update())
        // {
        //     if (Time::elapsed(lastAngleUpdate, 100))
        //     {
        //         lastAngleUpdate = Time::ms();
        //         horTopic.publish(aimSensor.getAngle());
        //     }
        // }

        if (Pins::getInput(firePin))
        {
            checkFire();
        }

        fogMachine.process();
    }

    void Handler::checkLoaded(uint16_t distance)
    {
        if (loaded) return;

        if (distance < 100)
        {
            loadedTime = Time::ms();
        }
        else
        {
            if (loadedTime)
            {
                if (Time::elapsed(loadedTime, 500))
                {
                    loadedTime = 0;
                    loaded = true;
                    loadedTopic.publish();
                }
            }
        }
    }

    void Handler::checkFire()
    {
        if (loaded)
        {
            loaded = false;
            firedTopic.publish();
            fogMachine.trigger();
        }
    }
}
