
#include "cannon.h"
#include "fogmachine.h"

extern "C" void app_main()
{
    TaskHandle_t fogMachineTaskHandle;
    xTaskCreatePinnedToCore(FogMachine::task, "FogMachine Task", 4096, NULL, 10, &fogMachineTaskHandle, 1);

    Cannon::Handler cannon{Cannon::Handler::ID::Left};
    cannon.init();

    while (1)
    {
        cannon.process();
    }
}
