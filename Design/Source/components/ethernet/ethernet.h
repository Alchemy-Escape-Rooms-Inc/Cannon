#pragma once

#include "esp_eth_driver.h"

namespace Ethernet
{
    bool init();
}

#define ESP_RETURN_FALSE_ON_ERROR(x, log_tag, format, ...) do {                                             \
        esp_err_t err_rc_ = (x);                                                                           \
        if (unlikely(err_rc_ != ESP_OK)) {                                                                 \
            ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__ __VA_OPT__(,) __VA_ARGS__);        \
            return false;                                                                                        \
        }                                                                                                  \
    } while(0)

#define ESP_RETURN_FALSE_ON_FALSE(a, err_code, log_tag, format, ...) do {                                   \
        if (unlikely(!(a))) {                                                                              \
            ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__ __VA_OPT__(,) __VA_ARGS__);        \
            return false;                                                                                        \
        }                                                                                                  \
    } while(0)
