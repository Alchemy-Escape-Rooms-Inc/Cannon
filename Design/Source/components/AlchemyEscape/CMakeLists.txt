idf_component_register(SRCS

"als31300/als31300.cpp"

"i2c/i2c.cpp"

"pins/pins.cpp"

"time/time.cpp"

"vl53l0x/vl53l0x.cpp"

"vl53l0x/core/src/vl53l0x_api_calibration.cpp"
"vl53l0x/core/src/vl53l0x_api_core.cpp"
"vl53l0x/core/src/vl53l0x_api_ranging.cpp"
"vl53l0x/core/src/vl53l0x_api_strings.cpp"
"vl53l0x/core/src/vl53l0x_api.cpp"

"vl53l0x/platform/src/vl53l0x_i2c_comms.cpp"
"vl53l0x/platform/src/vl53l0x_platform.cpp"

"vl6180x/VL6180X.cpp"

"wrapper/Arduino.cpp"
"wrapper/Wire.cpp"

INCLUDE_DIRS
"als31300/include"
"i2c/include"
"pins/include"
"time/include"
"vl53l0x/include"
"wrapper/include"

"vl6180x/inc"

REQUIRES driver esp_timer)
