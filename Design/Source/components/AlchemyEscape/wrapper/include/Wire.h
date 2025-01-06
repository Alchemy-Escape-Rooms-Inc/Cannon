#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <deque>

struct TwoWire
{
    void begin();
    void write(uint8_t byte);
    uint8_t read();
    void beginTransmission(uint8_t address);
    void endTransmission();
    void requestFrom(uint8_t address, uint8_t count);

    std::vector<uint8_t> writeBuffer;
    std::deque<uint8_t> readBuffer;
    uint8_t address = 0;
};

extern TwoWire Wire;