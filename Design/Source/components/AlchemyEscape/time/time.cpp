#include "time.h"

#include <esp_timer.h>

namespace Time
{
    int64_t ms()
    {
        return esp_timer_get_time() / 1000;
    }

    bool elapsed(int64_t last, size_t length)
    {
        return ms() - last >= length;
    }

    void wait(int64_t duration)
    {
        if (duration <= 0) return;

        int64_t time = ms();
        while (!elapsed(time, duration)) {}
    }
}
