#pragma once

#include <Arduino.h>

namespace TinyCon
{
    constexpr const char *TinyConVersion = "0.1";
    constexpr bool TinyConUSBEnabledByDefault = true;
    constexpr bool TinyConBLEEnabledByDefault = true;

    constexpr int MaxNativeAdcPinCount = 6;
    // 8 if using all 6 axis, 14 if we use all the axis pins for buttons
    constexpr int MaxNativeGpioPinCount = 14;
    constexpr Tiny::TILogLevel StateLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel BluetoothLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel GamepadLogLevel = Tiny::TILogLevel::Info;
    constexpr Tiny::TILogLevel I2CLogLevel = Tiny::TILogLevel::Info;
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
    #define USE_LC709203 0
    #define USE_NEOPIXEL 1
    #define USE_HAPTICTEST 0

    #define NO_USB 0
    #define NO_BLE 0
    #define NO_I2C_SLAVE 0
}