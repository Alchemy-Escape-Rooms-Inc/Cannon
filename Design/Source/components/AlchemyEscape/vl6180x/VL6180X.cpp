#include "VL6180X.h"

#include "i2c.h"
#include "time.h"

#include <esp_log.h>
#include <algorithm>

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
    if (!startRangeContinuous(40)) return;

    // Clear reset flag
    if (!writeByte(VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, 0x00)) return;

    // Reset success
    initialized = true;
}

void VL6180X::loadSettings()
{
    //https://www.st.com/resource/en/application_note/an4545-vl6180x-basic-ranging-application-note-stmicroelectronics.pdf
    // private settings from page 24 of app note
    // Mandatory : private registers
    writeByte(0x0207, 0x01);
    writeByte(0x0208, 0x01);
    writeByte(0x0096, 0x00);
    writeByte(0x0097, 0xfd);
    writeByte(0x00e3, 0x01);
    writeByte(0x00e4, 0x03);
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
    writeByte(0x0011, 0x10); // Enables polling for ‘New Sample ready’
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

    //Optional: Public registers - See data sheet for more detail
    writeByte(0x001b, 0x09); // Set default ranging inter-measurement
    // period to 100ms
    writeByte(0x003e, 0x31); // Set default ALS inter-measurement period
    // to 500ms
    writeByte(0x0014, 0x24); // Configures interrupt on ‘New Sample
    // Ready threshold event’ 
}

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

bool VL6180X::isInitialized()
{
    return initialized;
}

bool VL6180X::triggered()
{
    bool ret = Pins::getInput(intPin);
    if (ret) write8(VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0b001);

    uint8_t read;
    if (readByte(VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET, read)) // Added for debugging to see if chip was resetting. It isn't but the read adjusts timings enough that the sensor stops locking up.
    {
        if (read & 0x01) ESP_LOGE(TAG, "IN RESET!");
    }

    clearInterrupts();
    return ret;
}

void VL6180X::clearInterrupts()
{
    write8(VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0b111);
}

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
