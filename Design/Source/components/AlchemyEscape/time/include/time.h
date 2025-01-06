#pragma once

#include <cstdint>
#include <cstddef>

namespace Time
{
    int64_t ms();
    bool elapsed(int64_t last, size_t length);
}
