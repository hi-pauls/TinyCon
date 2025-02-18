#pragma once

#include "Config.h"

#include "GamepadController.h"
#include "Power.h"

#if USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif
#if USE_OLED
#include <Adafruit_SSD1306.h>
#endif
#include <Arduino.h>

#include <cstdint>

namespace TinyCon
{
class IndicatorController
{
public:
    enum class LedEffects : int8_t { Off = 0, On = 1, Pulse = 2, Fade = 3 };

    IndicatorController(TwoWire& masterI2c, const GamepadController& controller, const PowerController& power)
        : Controller(controller), Power(power)
#if USE_OLED
        , SSD1306{128, 32, &masterI2c, -1, 1000000}
#endif
        {}

    void Init();
    void Update(uint32_t deltaTime, char mode);

    void SetBlue(LedEffects effect) { NextBlueEffect = effect; }
    void SetRed(LedEffects effect) { NextRedEffect = effect; }

#if USE_NEOPIXEL
    void SetRgbRed(LedEffects effect) { NextRgbRedEffect = effect; }
    void SetRgbGreen(LedEffects effect) { NextRgbGreenEffect = effect; }
    void SetRgbBlue(LedEffects effect) { NextRgbBlueEffect = effect; }
#endif

    void Disable();

private:
    uint32_t LastUpdate = millis();
    const GamepadController& Controller;
    const PowerController& Power;

    // This LED is on, when BLE is on and connected, fading when we are advertising or off otherwise
    static constexpr int8_t BlueLedPin = LED_BLUE;
    int32_t BlueEffectTime = 0;
    LedEffects BlueEffect = LedEffects::Off;
    LedEffects NextBlueEffect = LedEffects::Off;
    uint8_t BlueEffectValue = 0;
    void UpdateBlue(uint32_t deltaTime);

    // This LED is on, when USB is on and connected and mounted.
    static constexpr int8_t RedLedPin = LED_RED;
    int32_t RedEffectTime = 0;
    LedEffects RedEffect = LedEffects::Off;
    LedEffects NextRedEffect = LedEffects::Off;
    uint8_t RedEffectValue = 0;
    void UpdateRed(uint32_t deltaTime);

    void UpdateLed(uint32_t deltaTime, uint8_t& value, int32_t& time, LedEffects& current, LedEffects& next);

    // Red for errors, Green for single-controller (USB and I2C only), Blue for dual-controller (USB and I2C only)
#if USE_NEOPIXEL
    Adafruit_NeoPixel RgbPixel{1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800};
    LedEffects RgbRedEffect = LedEffects::Off;
    LedEffects NextRgbRedEffect = LedEffects::Off;
    int32_t RgbRedEffectTime = 0;
    uint8_t RgbRedValue = 255;
    LedEffects RgbGreenEffect = LedEffects::Off;
    LedEffects NextRgbGreenEffect = LedEffects::Off;
    int32_t RgbGreenEffectTime = 0;
    uint8_t RgbGreenValue = 0;
    LedEffects RgbBlueEffect = LedEffects::Off;
    LedEffects NextRgbBlueEffect = LedEffects::Off;
    int32_t RgbBlueEffectTime = 0;
    uint8_t RgbBlueValue = 0;
    void SetRgb(uint8_t red, uint8_t green, uint8_t blue) { RgbPixel.setPixelColor(1, RgbPixel.Color(red, green, blue)); }
    void UpdateRgb(uint32_t deltaTime);
#endif

#if USE_OLED
    static constexpr auto ControllerHeight = 32 / GamepadController::MaxControllers;
    static constexpr auto AxisRoot = 16;
    static constexpr auto MpuWidth = 36;
    static constexpr auto MpuStart = 120 - 2 - MpuWidth;
    static constexpr auto ButtonStart = 34;
    static constexpr auto ButtonEnd = MpuStart - 2;
    static constexpr auto AccelerationScale = 10;
    static constexpr auto AngularVelocityScale = 10;
    static constexpr auto OrientationScale = 60;
    Adafruit_SSD1306 SSD1306;
    bool DisplayPresent = false;
    bool Suspended = false;
    void UpdateDisplay(char mode);
#endif
};
}