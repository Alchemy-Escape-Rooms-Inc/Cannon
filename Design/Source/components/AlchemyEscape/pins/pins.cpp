#include "pins.h"
#include <soc/gpio_reg.h>
#include <rom/gpio.h>

namespace Pins
{
    void initPin(gpio_num_t pin, gpio_mode_t mode, bool defaultHigh)
    {
        resetPin(pin);
        gpio_set_direction(pin, mode);
        if (mode == GPIO_MODE_OUTPUT) gpio_set_level(pin, defaultHigh);
        else if (defaultHigh) gpio_pulldown_en(pin);
    }

    void resetPin(gpio_num_t pin)
    {
        // Some pins are default configured and need to be reset to be used, it's not enough to just set pin direction and level
        gpio_reset_pin(pin); 
    }

    void init()
    {
        // Init pins here
        // initPin(GPIO_NUM_35, GPIO_MODE_OUTPUT);       // Set GPIO35 to output, default low
        // initPin(GPIO_NUM_36, GPIO_MODE_OUTPUT, true); // Set GPIO36 to output, default high
        // initPin(GPIO_NUM_37, GPIO_MODE_INPUT);        // Set GPIO37 to input
    }

    void setOutput(gpio_num_t pin, bool high)
    {
        gpio_set_level(pin, high);
    }

    bool getOutput(gpio_num_t pin)
    {
        if (pin >= 32) return GPIO_REG_READ(GPIO_OUT1_REG) & 1 << (pin - 32);
                  else return GPIO_REG_READ(GPIO_OUT_REG)  & 1 << pin;
    }

    bool getInput(gpio_num_t pin)
    {
        return gpio_get_level(pin);
    }

    bool DebouncedInput::debounce(gpio_num_t pin)
    {
        uint8_t currentRead = getInput(pin);

        debounceCount = (currentRead != settledRead && currentRead == previousRead) ? debounceCount + 1 : 0;
        previousRead = currentRead;
        
        if (debounceCount == DEBOUNCE_FILTER)
        {
            settledRead = currentRead;
            return true;
        }
        
        return false;
    }

    bool DebouncedInput::get()
    {
        return settledRead;
    }
}