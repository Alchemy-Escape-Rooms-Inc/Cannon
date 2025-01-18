#pragma once

#include "VL6180X_defs.h"

#include "Arduino.h"
#include "Wire.h"

#include "pins.h"

class VL6180X
{
    static constexpr const char* TAG = "VL6180X";
    gpio_num_t enablePin;
    gpio_num_t intPin;
    uint8_t address;

public:
    static constexpr uint8_t DEFAULT_I2C_ADDR = 0x29;

    VL6180X(gpio_num_t enablePin, gpio_num_t intPin, uint8_t address = DEFAULT_I2C_ADDR);
    ~VL6180X();

    bool initialized = false;

    void init();
    bool isInitialized();
    bool triggered();

private: 
    void loadSettings(void);
    bool startRangeContinuous(uint16_t period_ms = 50);
    void clearInterrupts();
    
    void write8(uint16_t reg, uint8_t data);
    void write16(uint16_t reg, uint16_t data);
    uint16_t read16(uint16_t reg);
    uint8_t read8(uint16_t reg);

    bool updateByte(uint16_t reg, uint8_t andData, uint8_t orData);
    bool readByte(uint16_t reg, uint8_t& data);
    bool readWord(uint16_t reg, uint16_t& data);
    bool writeByte(uint16_t reg, uint8_t data);
    bool writeWord(uint16_t reg, uint16_t data);
};
