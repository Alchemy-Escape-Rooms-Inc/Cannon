#pragma once

#include <cstdint>
#include <cstddef>

// https://www.allegromicro.com/-/media/files/datasheets/als31300-datasheet.ashx

namespace ALS31300
{
    class Sensor
    {
        // See Customer Write Access
        static constexpr uint32_t customerAccessCode = 0x2C413534;
        static constexpr uint32_t customerAccessRegister = 0x35;

        bool write(uint8_t reg, uint32_t value);
        bool read(uint8_t reg, uint32_t& value);

    public:
        Sensor() {}
        Sensor(uint8_t address);
        ~Sensor();

        void init(uint8_t address);

        bool update();
        uint16_t getAngle();

        bool programAddress(uint8_t newAddress);

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        uint8_t address = 0;

    private:
        float avgAngle = 0.0f;

        float angleFromXY(float x, float y);
        void xyFromAngle(float angle, float& x, float& y);
    };
}