#include "als31300.h"
#include "registers.h"

#include "i2c.h"

#include <cstdio>
#include <cmath>

namespace ALS31300
{
    Sensor::Sensor(uint8_t address)
    {
        init(address);
    }

    void Sensor::init(uint8_t address)
    {
        address = address & 0x7F;
        this->address = address;

        I2C::registerDevice(address);
    }

    Sensor::~Sensor()
    {
        I2C::unregisterDevice(address);
    }

    bool Sensor::update()
    {
        uint16_t newX = 0;
        uint16_t newY = 0;
        uint16_t newZ = 0;

        uint32_t readData;
        if (!read(0x28, readData)) return false;
        Register0x28 reg28{readData};

        newX = reg28.xAxisMsbs << 8;
        newY = reg28.yAxisMsbs << 8;
        newZ = reg28.zAxisMsbs << 8;

        if (!read(0x29, readData)) return false;
        Register0x29 reg29{readData};

        newX |= reg29.xAxisLsbs;
        newY |= reg29.yAxisLsbs;
        newZ |= reg29.zAxisLsbs;

        volatile static float filterIntensity = 8.0f;
        x = (float((int16_t) newX) + x * (filterIntensity - 1)) / filterIntensity;
        y = (float((int16_t) newY) + y * (filterIntensity - 1)) / filterIntensity;
        z = (float((int16_t) newZ) + z * (filterIntensity - 1)) / filterIntensity;

        return true;
    }

    bool Sensor::programAddress(uint8_t newAddress)
    {
        newAddress &= 0x7F;
        
        // Enter Customer Access Mode to enable register writes
        if (!write(customerAccessRegister, customerAccessCode)) return false;

        // Read register
        uint32_t readData;
        if (!read(0x02, readData)) return false;

        // Update address
        Register0x02 registerData = {readData};
        registerData.slaveAddress = newAddress;

        // Write new address
        if (!write(0x02, registerData.raw)) return false;

        printf("Address programming successful! Power cycle device to check.\n");
        
        return true;
    }

    bool Sensor::write(uint8_t reg, uint32_t value)
    {
        uint8_t sendPayload[5] =
        {
            reg,
            uint8_t(value >> 24 & 0xFF),
            uint8_t(value >> 16 & 0xFF),
            uint8_t(value >>  8 & 0xFF),
            uint8_t(value >>  0 & 0xFF)
        };

        return I2C::write(address, sendPayload, 5);
    }

    bool Sensor::read(uint8_t reg, uint32_t& value)
    {
        uint8_t sendPayload = reg;
        uint8_t receivePayload[4];

        if (!I2C::exchange(address, &sendPayload, 1, receivePayload, 4)) return false;

        value = receivePayload[0] << 24 |
                receivePayload[1] << 16 |
                receivePayload[2] <<  8 |
                receivePayload[3];

        return true;
    }

    uint16_t Sensor::getAngle()
    {
        // Convert new reading to angle and then to x,y
        float currentAngle = angleFromXY(x, y);
        float currentX, currentY;
        xyFromAngle(currentAngle, currentX, currentY);

        // Convert stored avgAngle to x,y
        float avgX, avgY;
        xyFromAngle(avgAngle, avgX, avgY);

        // Compute difference between current and avg
        float dx = currentX - avgX;
        float dy = currentY - avgY;
        float dist = sqrtf(dx * dx + dy * dy);

        // Choose alpha in [0, alphaMax], based on distance
        // The scaleFactor controls how quickly alpha increases with distance.
        // For small changes, alpha is small; for large changes, alpha can max out.
        volatile static float scaleFactor = 4.5f;
        volatile static float alphaMax    = 0.5f;

        float alpha = dist * scaleFactor;
        if (alpha > alphaMax) alpha = alphaMax;
        
        // Blend the average
        avgX += alpha * dx;
        avgY += alpha * dy;

        // Convert back to an angle
        avgAngle = angleFromXY(avgX, avgY);

        // Return integer angle
        return static_cast<uint16_t>(avgAngle + 0.5f);
    }

    float Sensor::angleFromXY(float x, float y)
    {
        constexpr float PI = float(M_PI);
        constexpr float RAD_TO_DEG (180.0f / PI);

        float angle = atan2f(y, x) * RAD_TO_DEG;
        if (angle < 0) angle += 360.0f;
        
        return angle;
    }

    void Sensor::xyFromAngle(float angle, float& x, float& y)
    {
        constexpr float PI = float(M_PI);
        constexpr float DEG_TO_RAD (PI / 180.0f);

        x = cosf(angle * DEG_TO_RAD);
        y = sinf(angle * DEG_TO_RAD);
    }
}
