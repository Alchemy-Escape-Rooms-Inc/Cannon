#pragma once

// Define some additional registers mentioned in application notes and we use
///! period between each measurement when in continuous mode
#define SYSRANGE__INTERMEASUREMENT_PERIOD 0x001b // P19 application notes

#define GPIOx_FUNCTIONALITY_SELECT_SHIFT          1
#define GPIOx_POLARITY_SELECT_MASK              0x20
#define SYSTEM_MODE_GPIO1                       0x011
#define GPIOx_SELECT_GPIO_INTERRUPT_OUTPUT      0x08
#define CONFIG_GPIO_INTERRUPT_LEVEL_LOW        0x01
#define CONFIG_GPIO_RANGE_SHIFT                0
#define CONFIG_GPIO_RANGE_MASK                 (0x7<<CONFIG_GPIO_RANGE_SHIFT)
#define SYSTEM_INTERRUPT_CONFIG_GPIO           0x014

#define GPIOx_SELECT_GPIO_INTERRUPT_OUTPUT     0x08

/**
 * @def SYSRANGE_THRESH_HIGH
 * High level range  threshold (must be scaled)
 * @ingroup device_regdef
 */
#define SYSRANGE_THRESH_HIGH                  0x019

/**
 * @def SYSRANGE_THRESH_LOW
 * Low level range  threshold (must be scaled)
 * @ingroup device_regdef
 */
#define SYSRANGE_THRESH_LOW                   0x01A

/** mask existing bit in #SYSRANGE_START*/
#define SYSRANGE_START_MODE_MASK          0x03
/** bit 0 in #SYSRANGE_START write 1 toggle state in continuous mode and arm next shot in single shot mode */
#define MODE_START_STOP                   0x01
/** bit 1 write 1 in #SYSRANGE_START set continuous operation mode */
#define MODE_CONTINUOUS                   0x02
/** bit 1 write 0 in #SYSRANGE_START set single shot mode */
#define MODE_SINGLESHOT                   0x00

///! Device model identification number
#define VL6180X_REG_IDENTIFICATION_MODEL_ID 0x000

#define GPIOx_SELECT_GPIO_INTERRUPT_OUTPUT 0x08
#define VL6180X_REG_SYSTEM_MODE_GPIO1 0x011
#define CONFIG_GPIO_INTERRUPT_LEVEL_LOW        0x01

///! Interrupt configuration
#define VL6180X_REG_SYSTEM_INTERRUPT_CONFIG 0x014
///! Interrupt clear bits
#define VL6180X_REG_SYSTEM_INTERRUPT_CLEAR 0x015
///! Fresh out of reset bit
#define VL6180X_REG_SYSTEM_FRESH_OUT_OF_RESET 0x016
///! Trigger Ranging
#define VL6180X_REG_SYSRANGE_START 0x018
///! Part to part range offset
#define VL6180X_REG_SYSRANGE_PART_TO_PART_RANGE_OFFSET 0x024
///! Trigger Lux Reading
#define VL6180X_REG_SYSALS_START 0x038
///! Lux reading gain
#define VL6180X_REG_SYSALS_ANALOGUE_GAIN 0x03F
///! Integration period for ALS mode, high byte
#define VL6180X_REG_SYSALS_INTEGRATION_PERIOD_HI 0x040
///! Integration period for ALS mode, low byte
#define VL6180X_REG_SYSALS_INTEGRATION_PERIOD_LO 0x041
///! Specific error codes
#define VL6180X_REG_RESULT_RANGE_STATUS 0x04d
///! Interrupt status
#define VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO 0x04f
///! Light reading value
#define VL6180X_REG_RESULT_ALS_VAL 0x050
///! Ranging reading value
#define VL6180X_REG_RESULT_RANGE_VAL 0x062
///! I2C Slave Device Address
#define VL6180X_REG_SLAVE_DEVICE_ADDRESS 0x212

#define VL6180X_ALS_GAIN_1 0x06    ///< 1x gain
#define VL6180X_ALS_GAIN_1_25 0x05 ///< 1.25x gain
#define VL6180X_ALS_GAIN_1_67 0x04 ///< 1.67x gain
#define VL6180X_ALS_GAIN_2_5 0x03  ///< 2.5x gain
#define VL6180X_ALS_GAIN_5 0x02    ///< 5x gain
#define VL6180X_ALS_GAIN_10 0x01   ///< 10x gain
#define VL6180X_ALS_GAIN_20 0x00   ///< 20x gain
#define VL6180X_ALS_GAIN_40 0x07   ///< 40x gain

#define VL6180X_ERROR_NONE 0        ///< Success!
#define VL6180X_ERROR_SYSERR_1 1    ///< System error
#define VL6180X_ERROR_SYSERR_5 5    ///< Sysem error
#define VL6180X_ERROR_ECEFAIL 6     ///< Early convergence estimate fail
#define VL6180X_ERROR_NOCONVERGE 7  ///< No target detected
#define VL6180X_ERROR_RANGEIGNORE 8 ///< Ignore threshold check failed
#define VL6180X_ERROR_SNR 11        ///< Ambient conditions too high
#define VL6180X_ERROR_RAWUFLOW 12   ///< Raw range algo underflow
#define VL6180X_ERROR_RAWOFLOW 13   ///< Raw range algo overflow
#define VL6180X_ERROR_RANGEUFLOW 14 ///< Raw range algo underflow
#define VL6180X_ERROR_RANGEOFLOW 15 ///< Raw range algo overflow