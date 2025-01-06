
#include "cannon.h"

extern "C" void app_main()
{
    Cannon::Handler cannon{Cannon::Handler::ID::Left};
    cannon.init();

    while (1)
    {
        cannon.process();
    }
}
