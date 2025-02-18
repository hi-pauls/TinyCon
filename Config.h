#pragma once

#include <Arduino.h>

namespace TinyCon
{
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

#define TINYCON_DEFAULT_USB_ENABLED true
#define TINYCON_DEFAULT_BLE_ENABLED true

#define USE_OLED 1
#define USE_LC709203 0
#define USE_NEOPIXEL 0
#define USE_HAPTICTEST 0

#define NO_USB 0
#define NO_BLE 0
    #define NO_I2C_SLAVE 0
}
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))