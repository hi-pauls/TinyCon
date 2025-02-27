#pragma once

#include "Core/Utilities/TILog.h"

#include <Arduino.h>

namespace TinyCon
{
    constexpr const char *TinyConVersion = "0.1";
    constexpr bool TinyConUSBEnabledByDefault = true;
    constexpr bool TinyConBLEEnabledByDefault = true;

    constexpr int MaxNativeAdcPinCount = 6;
    // 8 if using all 6 axis, 14 if we use all the axis pins for buttons
    constexpr int MaxNativeGpioPinCount = 14;
    constexpr int MaxI2CWriteBufferFill = SERIAL_BUFFER_SIZE;

    // Not the prettiest way to do logging for now, but should do the job
    constexpr Tiny::TILogLevel StateLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel BluetoothLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel GamepadLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel I2CLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel CommandLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel HapticLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel IndicatorLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel UsbLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel PowerLogLevel = Tiny::TILogLevel::Info;

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

    #define USE_OLED 1
    #define USE_HAPTICTEST 0

// Some example configs for feather boards
#if defined(ADAFRUIT_FEATHER_ESP32S2)
    #define USE_NEOPIXEL 1
    #define USE_LC709203 1
#elif defined(ADAFRUIT_FEATHER_ESP32S3)
    #define USE_NEOPIXEL 1
    #define USE_LC709203 1
#elif defined(ADAFRUIT_FEATHER_NRF52840)
    #define USE_NEOPIXEL 1
    #define USE_LC709203 0
#elif defined(ADAFRUIT_FEATHER_NRF52832)
    #define USE_NEOPIXEL 0
    #define USE_LC709203 0
#else
    #define USE_NEOPIXEL 0
    #define USE_LC709203 0
#endif

#ifdef USE_TINYUSB
    #define NO_USB 0
#else
    #define NO_USB 1
#endif

    #define NO_BLE 0
    #define NO_I2C_SLAVE 0
}
