#include "i2c.h"

#include <algorithm>
#include <vector>

#include <driver/i2c_master.h>

#define I2C_SCL GPIO_NUM_18
#define I2C_SDA GPIO_NUM_16

namespace I2C
{
    struct I2CDevice
    {
        uint8_t address;
        i2c_master_dev_handle_t deviceHandle;
    };

    namespace
    {
        bool initialized = false;

        i2c_master_bus_handle_t busHandle;
        std::vector<I2CDevice> slaves;

        constexpr uint32_t I2C_CLK_SPEED = 100000; // 100KHz to better deal with long cable runs
    }

    void init()
    {
        if (initialized) return;

        constexpr i2c_master_bus_config_t i2cBusConfig =
        {
            .i2c_port = -1,
            .sda_io_num = I2C_SDA,
            .scl_io_num = I2C_SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
        };

        ESP_ERROR_CHECK(i2c_new_master_bus(&i2cBusConfig, &busHandle));
        initialized = true;
    }

    bool getDevice(uint8_t address, i2c_master_dev_handle_t& handle)
    {
        for (int i = 0; i < slaves.size(); i++)
        {
            if (slaves[i].address == address)
            {
                handle = slaves[i].deviceHandle;
                return true;
            }
        }
        return false;
    }

    bool registerDevice(uint8_t address)
    {
        if (!initialized) return false;

        i2c_device_config_t masterConfig =
        {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = uint8_t(address & 0x7F),
            .scl_speed_hz = I2C_CLK_SPEED,
        };

        i2c_master_dev_handle_t deviceHandle;
        esp_err_t err = i2c_master_bus_add_device(busHandle, &masterConfig, &deviceHandle);
        if (err != ESP_OK) return false;
        
        slaves.push_back(I2CDevice(address, deviceHandle));
        return true;
    }

    bool unregisterDevice(uint8_t address)
    {
        if (!initialized) return false;

        i2c_master_dev_handle_t deviceHandle;
        if (!getDevice(address, deviceHandle)) return true; // No need to unregister, return true

        esp_err_t err = i2c_master_bus_rm_device(deviceHandle);
        if (err != ESP_OK) return false;
        
        std::erase_if(slaves, [&](const I2CDevice& device) -> bool
        {
            return device.address == address;
        });

        return true;
    }

    bool changeAddress(uint8_t oldAddress, uint8_t newAddress)
    {
        if (!initialized) return false;

        i2c_master_dev_handle_t handle;
        if (getDevice(newAddress, handle)) return false; // Address exists, abort

        if (!unregisterDevice(oldAddress)) return false;
        if (!registerDevice(newAddress)) return false;

        return true;
    }

    bool devicePresent(uint8_t address)
    {
        if (!initialized) return false;

        return i2c_master_probe(busHandle, address, 10) == ESP_OK;
    }

    bool exchange(uint8_t address, uint8_t* sendPayload, size_t sendSize, uint8_t* receivePayload, size_t receiveSize)
    {
        if (!initialized) return false;

        i2c_master_dev_handle_t deviceHandle;
        if (!getDevice(address, deviceHandle)) return false;

        esp_err_t err = i2c_master_transmit_receive(deviceHandle, sendPayload, sendSize, receivePayload, receiveSize, 10);
        return err == ESP_OK;
    }

    bool read(uint8_t address, uint8_t* receivePayload, size_t receiveSize)
    {
        if (!initialized) return false;

        i2c_master_dev_handle_t deviceHandle;
        if (!getDevice(address, deviceHandle)) return false;

        esp_err_t err = i2c_master_receive(deviceHandle, receivePayload, receiveSize, 10);
        return err == ESP_OK;
    }

    bool write(uint8_t address, uint8_t* sendPayload, size_t sendSize)
    {
        if (!initialized) return false;

        i2c_master_dev_handle_t deviceHandle;
        if (!getDevice(address, deviceHandle)) return false;

        esp_err_t err = i2c_master_transmit(deviceHandle, sendPayload, sendSize, 10);
        return err == ESP_OK;
    }
}
