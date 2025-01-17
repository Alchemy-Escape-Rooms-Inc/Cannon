#include "VL6180X.h"
#include "i2c.h"
#include "time.h"


#include <algorithm>

bool VL6180X::startRangeContinuous(uint16_t period_ms)
{
    uint8_t period = std::clamp((period_ms / 10) - 1, 0, 0xFD);

    // Set  ranging inter-measurement
    if (!writeByte(SYSRANGE__INTERMEASUREMENT_PERIOD, period)) return false;

    // Set Thresholds
    if (!writeByte(SYSRANGE_THRESH_HIGH, 255)) return false;
    if (!writeByte(SYSRANGE_THRESH_LOW, 100)) return false;
    
    // Set interrupt mode
    if (!updateByte(SYSTEM_INTERRUPT_CONFIG_GPIO, (uint8_t) (~CONFIG_GPIO_RANGE_MASK), CONFIG_GPIO_INTERRUPT_LEVEL_LOW)) return false;
	
    // Setup GPIO1
	if (!writeByte(SYSTEM_MODE_GPIO1, GPIOx_POLARITY_SELECT_MASK | (GPIOx_SELECT_GPIO_INTERRUPT_OUTPUT << GPIOx_FUNCTIONALITY_SELECT_SHIFT))) return false;

    // Start a continuous range measurement
    if (!writeByte(VL6180X_REG_SYSRANGE_START, MODE_START_STOP | MODE_CONTINUOUS)) return false;

    return true;
}

VL6180X::~VL6180X()
{
    I2C::unregisterDevice(address);
}

VL6180X::VL6180X(gpio_num_t enablePin, gpio_num_t intPin, uint8_t address) : enablePin(enablePin), intPin(intPin), address(address) {}

void VL6180X::init()
{
    // Reset VL6180X
    Pins::initPin(intPin, GPIO_MODE_INPUT);
    Pins::initPin(enablePin, GPIO_MODE_OUTPUT);
    
    Pins::setOutput(enablePin, false);    
    Time::wait(100);
    Pins::setOutput(enablePin, true);
    Time::wait(100);

    // Register I2C Device
    if (!I2C::registerDevice(address)) return;

    // Validate model and reset state
    uint8_t read;

    if (!readByte(VL6180X_REG_IDENTIFICATION_MODEL_ID, read)) return;
    if (read != 0xB4) return;

    if (!readByte(VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, read)) return;
    if (!(read & 0x01)) return;
    
    // Load default config
    loadSettings();
    
    // Enable continous mode with GPIO interrupt
    if (!startRangeContinuous(20)) return;

    // Clear reset flag
    if (!writeByte(VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, 0x00)) return;

    // Reset success
    initialized = true;
}

bool VL6180X::isInitialized()
{
    return initialized;
}

bool VL6180X::triggered()
{
    bool ret = Pins::getInput(intPin);
    clearInterrupts(); 

    return ret;
}

// void VL6180X::setAddress(uint8_t newAddr)
// {
//     write8(VL6180X_REG_SLAVE_DEVICE_ADDRESS, newAddr & 0x7F);

//     I2C::unregisterDevice(address);
//     address = newAddr;
//     I2C::registerDevice(address);
// }

// uint8_t VL6180X::getAddress() { return address; }

void VL6180X::loadSettings()
{
    // private settings from page 24 of app note
    writeByte(0x0207, 0x01);
    writeByte(0x0208, 0x01);
    writeByte(0x0096, 0x00);
    writeByte(0x0097, 0xfd);
    writeByte(0x00e3, 0x00);
    writeByte(0x00e4, 0x04);
    writeByte(0x00e5, 0x02);
    writeByte(0x00e6, 0x01);
    writeByte(0x00e7, 0x03);
    writeByte(0x00f5, 0x02);
    writeByte(0x00d9, 0x05);
    writeByte(0x00db, 0xce);
    writeByte(0x00dc, 0x03);
    writeByte(0x00dd, 0xf8);
    writeByte(0x009f, 0x00);
    writeByte(0x00a3, 0x3c);
    writeByte(0x00b7, 0x00);
    writeByte(0x00bb, 0x3c);
    writeByte(0x00b2, 0x09);
    writeByte(0x00ca, 0x09);
    writeByte(0x0198, 0x01);
    writeByte(0x01b0, 0x17);
    writeByte(0x01ad, 0x00);
    writeByte(0x00ff, 0x05);
    writeByte(0x0100, 0x05);
    writeByte(0x0199, 0x05);
    writeByte(0x01a6, 0x1b);
    writeByte(0x01ac, 0x3e);
    writeByte(0x01a7, 0x1f);
    writeByte(0x0030, 0x00);

    // Recommended : Public registers - See data sheet for more detail
    writeByte(0x0011, 0x10); // Enables polling for 'New Sample ready'
                      // when measurement completes
    writeByte(0x010a, 0x30); // Set the averaging sample period
                      // (compromise between lower noise and
                      // increased execution time)
    writeByte(0x003f, 0x46); // Sets the light and dark gain (upper
                      // nibble). Dark gain should not be
                      // changed.
    writeByte(0x0031, 0xFF); // sets the # of range measurements after
                      // which auto calibration of system is
                      // performed
    writeByte(0x0041, 0x63); // Set ALS integration time to 100ms
    writeByte(0x002e, 0x01); // perform a single temperature calibration
                      // of the ranging sensor

    // Optional: Public registers - See data sheet for more detail
    writeByte(SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09);         // Set default ranging inter-measurement
                      // period to 100ms
    writeByte(0x003e, 0x31); // Set default ALS inter-measurement period
                      // to 500ms
    writeByte(VL6180X_REG_SYSTEM_INTERRUPT_CONFIG, 0x24); // Configures interrupt on 'New Sample
                      // Ready threshold event'
}

// VL6180X::ReadStatus VL6180X::readRange(uint8_t& range)
// {
//     uint8_t read;

//     // wait for device to be ready for range measurement
//     if (!readByte(VL6180X_REG_RESULT_RANGE_STATUS, read)) return ReadStatus::BadRangeStatusRead;
//     if (!(read & 1)) return ReadStatus::BadRangeStatusValue;

//     // Start a range measurement
//     write8(VL6180X_REG_SYSRANGE_START, 0x01);

//     int64_t ms = Time::ms();
    
//     read = 0;
//     while (!(read & 0b100))
//     {
//         if (!readByte(VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO, read)) return ReadStatus::BadInterruptStatusRead;
//         if (Time::elapsed(ms, 500)) return ReadStatus::GetReadReadyTimeout;
//     }

//     if (!readByte(VL6180X_REG_RESULT_RANGE_VAL, range)) return ReadStatus::BadRangeRead;
//     clearInterrupts();

//     return ReadStatus::ReadOkay;
// }

void VL6180X::clearInterrupts()
{
    write8(VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0b111);
}

// boolean VL6180X::startRange()
// {
//     // wait for device to be ready for range measurement
//     while (!(read8(VL6180X_REG_RESULT_RANGE_STATUS) & 0x01));

//     // Start a range measurement
//     write8(VL6180X_REG_SYSRANGE_START, 0x01);

//     return true;
// }

// boolean VL6180X::isRangeComplete()
// {
//     // Poll until bit 2 is set
//     if ((read8(VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO) & 0x04))
//     return true;

//     return false;
// }

// boolean VL6180X::waitRangeComplete()
// {
//     // Poll until bit 2 is set
//     while (!(read8(VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO) & 0x04));

//     return true;
// }

// uint8_t VL6180X::readRangeResult()
// {
//     // read range in mm
//     uint8_t range = read8(VL6180X_REG_RESULT_RANGE_VAL);

//     // clear interrupt
//     clearInterrupts();

//     return range;
// }

// void VL6180X::stopRangeContinuous(void)
// {
//     // stop the continuous range operation, by setting the range register
//     // back to 1, Page 7 of appication notes
//     write8(VL6180X_REG_SYSRANGE_START, 0x01);
// }

// uint8_t VL6180X::readRangeStatus(void)
// {
//     return (read8(VL6180X_REG_RESULT_RANGE_STATUS) >> 4);
// }

// float VL6180X::readLux(uint8_t gain)
// {
//     uint8_t reg;

//     reg = read8(VL6180X_REG_SYSTEM_INTERRUPT_CONFIG);
//     reg &= ~0x38;
//     reg |= (0x4 << 3); // IRQ on ALS ready
//     write8(VL6180X_REG_SYSTEM_INTERRUPT_CONFIG, reg);

//     // 100 ms integration period
//     write8(VL6180X_REG_SYSALS_INTEGRATION_PERIOD_HI, 0);
//     write8(VL6180X_REG_SYSALS_INTEGRATION_PERIOD_LO, 100);

//     // analog gain
//     if (gain > VL6180X_ALS_GAIN_40)
//     {
//         gain = VL6180X_ALS_GAIN_40;
//     }

//     write8(VL6180X_REG_SYSALS_ANALOGUE_GAIN, 0x40 | gain);

//     // start ALS
//     write8(VL6180X_REG_SYSALS_START, 0x1);

//     // Poll until "New Sample Ready threshold event" is set
//     while (4 != ((read8(VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO) >> 3) & 0x7));

//     // read lux!
//     float lux = read16(VL6180X_REG_RESULT_ALS_VAL);

//     // clear interrupt
//     write8(VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07);

//     lux *= 0.32; // calibrated count/lux
//     switch (gain)
//     {
//         case VL6180X_ALS_GAIN_1:
//             break;
//         case VL6180X_ALS_GAIN_1_25:
//             lux /= 1.25;
//             break;
//         case VL6180X_ALS_GAIN_1_67:
//             lux /= 1.67;
//             break;
//         case VL6180X_ALS_GAIN_2_5:
//             lux /= 2.5;
//             break;
//         case VL6180X_ALS_GAIN_5:
//             lux /= 5;
//             break;
//         case VL6180X_ALS_GAIN_10:
//             lux /= 10;
//             break;
//         case VL6180X_ALS_GAIN_20:
//             lux /= 20;
//             break;
//         case VL6180X_ALS_GAIN_40:
//             lux /= 40;
//             break;
//     }

//     lux *= 100;
//     lux /= 100; // integration time in ms

//     return lux;
// }

// void VL6180X::setOffset(uint8_t offset)
// {
//     // write the offset
//     write8(VL6180X_REG_SYSRANGE_PART_TO_PART_RANGE_OFFSET, offset);
// }

// void VL6180X::getID(uint8_t *id_ptr)
// {
//     id_ptr[0] = read8(VL6180X_REG_IDENTIFICATION_MODEL_ID + 0);
//     id_ptr[1] = read8(VL6180X_REG_IDENTIFICATION_MODEL_ID + 1);
//     id_ptr[2] = read8(VL6180X_REG_IDENTIFICATION_MODEL_ID + 2);
//     id_ptr[3] = read8(VL6180X_REG_IDENTIFICATION_MODEL_ID + 3);
//     id_ptr[4] = read8(VL6180X_REG_IDENTIFICATION_MODEL_ID + 4);
//     id_ptr[6] = read8(VL6180X_REG_IDENTIFICATION_MODEL_ID + 6);
//     id_ptr[7] = read8(VL6180X_REG_IDENTIFICATION_MODEL_ID + 7);
// }

[[nodiscard]] bool VL6180X::readByte(uint16_t reg, uint8_t& value)
{
    uint8_t sendBuffer[2];
    sendBuffer[0] = reg >> 8;
    sendBuffer[1] = reg & 0xFF;

    uint8_t receiveBuffer;
    if (!I2C::exchange(address, sendBuffer, 2, &receiveBuffer, 1)) return false;
    value = receiveBuffer;
    return true;
}

[[nodiscard]] bool VL6180X::readWord(uint16_t reg, uint16_t& value)
{
    uint8_t sendBuffer[2];
    sendBuffer[0] = reg >> 8;
    sendBuffer[1] = reg & 0xFF;

    uint8_t receiveBuffer[2];

    if (!I2C::exchange(address, sendBuffer, 2, receiveBuffer, 2)) return false;
    value = receiveBuffer[0] << 8 | receiveBuffer[1];
    return true;
}

[[nodiscard]] bool VL6180X::writeByte(uint16_t reg, uint8_t data)
{
    uint8_t buffer[3];
    buffer[0] = uint8_t(reg >> 8);
    buffer[1] = uint8_t(reg & 0xFF);
    buffer[2] = data;

    return I2C::write(address, buffer, 3);
}

[[nodiscard]] bool VL6180X::writeWord(uint16_t reg, uint16_t data)
{
    uint8_t buffer[4];
    buffer[0] = uint8_t(reg >> 8);
    buffer[1] = uint8_t(reg & 0xFF);
    buffer[2] = uint8_t(data >> 8);
    buffer[3] = uint8_t(data & 0xFF);

    return I2C::write(address, buffer, 4);
}

[[nodiscard]] bool VL6180X::updateByte(uint16_t reg, uint8_t andData, uint8_t orData)
{
    uint8_t data;
    if (!readByte(reg, data)) return false;
    data = (data & andData) | orData;
    
    return writeByte(reg, data);
}

uint8_t VL6180X::read8(uint16_t reg)
{
    uint8_t read;
    (void) readByte(reg, read);

    return read;
}

uint16_t VL6180X::read16(uint16_t reg)
{
    uint16_t read;
    (void) readWord(reg, read);

    return read;
}

void VL6180X::write8(uint16_t reg, uint8_t data)
{
    (void) writeByte(reg, data);
}

void VL6180X::write16(uint16_t reg, uint16_t data)
{
    (void) writeWord(reg, data);
}
