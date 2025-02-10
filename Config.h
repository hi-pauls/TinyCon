#pragma once

#include <Arduino.h>

#define TINYCON_VERSION "0.1"
#ifndef TINYCON_PRODUCT
#define TINYCON_PRODUCT "TinyCon"
#endif
#ifndef TINYCON_VENDOR
#define TINYCON_VENDOR "Hel Industries"
#endif

#ifndef LED_BLUE
#define LED_BLUE -1
#endif
#ifndef LED_RED
#define LED_RED -1
#endif

#define TINYCON_DEFAULT_USB_ENABLED true
#define TINYCON_DEFAULT_BLE_ENABLED true

#define USE_OLED 1
#define USE_LC709203 0
#define USE_NEOPIXEL 0
#define USE_HAPTICTEST 0

#define NO_USB 0
#define NO_BLE 0
#define NO_SLAVE 0

#define LOG_LEVEL_STATE 1
#define LOG_LEVEL_CONNECTIONS 2
#define LOG_LEVEL_DETAILS 3
#define LOG_LEVEL_VERBOSE 4
#define LOG_LEVEL LOG_LEVEL_CONNECTIONS

#if LOG_LEVEL >= LOG_LEVEL_STATE
#define LOG_STATE(...) Serial.print(__VA_ARGS__)
#define LOG_STATE_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_STATE(...)
#define LOG_STATE_LN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_CONNECTIONS
#define LOG_BLUETOOTH(...) Serial.print(__VA_ARGS__)
#define LOG_BLUETOOTH_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_BLUETOOTH(...)
#define LOG_BLUETOOTH_LN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_CONNECTIONS
#define LOG_USB(...) Serial.print(__VA_ARGS__)
#define LOG_USB_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_USB(...)
#define LOG_USB_LN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_CONNECTIONS
#define LOG_I2C(...) Serial.print(__VA_ARGS__)
#define LOG_I2C_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_I2C(...)
#define LOG_I2C_LN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_CONNECTIONS
#define LOG_POWER(...) Serial.print(__VA_ARGS__)
#define LOG_POWER_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_POWER(...)
#define LOG_POWER_LN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DETAILS
#define LOG_CONTROLLER(...) Serial.print(__VA_ARGS__)
#define LOG_CONTROLLER_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_CONTROLLER(...)
#define LOG_CONTROLLER_LN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LOG_DISPLAY(...) Serial.print(__VA_ARGS__)
#define LOG_DISPLAY_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_DISPLAY(...)
#define LOG_DISPLAY_LN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LOG_I2C_DEVS(...) Serial.print(__VA_ARGS__)
#define LOG_I2C_DEVS_LN(...) Serial.println(__VA_ARGS__)
#else
#define LOG_I2C_DEVS(...)
#define LOG_I2C_DEVS_LN(...)
#endif

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
