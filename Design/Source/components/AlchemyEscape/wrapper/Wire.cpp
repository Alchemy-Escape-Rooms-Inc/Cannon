#include "Wire.h"

#include "i2c.h"

TwoWire Wire;

void TwoWire::begin()
{
    I2C::init();
}

void TwoWire::write(uint8_t byte)
{
    writeBuffer.push_back(byte);
}

uint8_t TwoWire::read()
{
    if (readBuffer.size())
    {
        uint8_t read = readBuffer[0];
        readBuffer.pop_front();
        return read;
    }

    return 0;
}

void TwoWire::beginTransmission(uint8_t address)
{
    this->address = address;
    I2C::registerDevice(address);
}

void TwoWire::endTransmission()
{
    I2C::write(address, writeBuffer.data(), writeBuffer.size());
}

void TwoWire::requestFrom(uint8_t address, uint8_t count)
{
    uint8_t* readBytes = new uint8_t[count];
    I2C::read(address, readBytes, count);

    for (int i = 0; i < count; i++)
    {
        readBuffer.push_back(readBytes[i]);
    }

    delete[] readBytes;
}