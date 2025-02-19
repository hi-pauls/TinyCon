#pragma once

#include "Config.h"

#include <Arduino.h>
#include <cstdint>
#include <Wire.h>

#if USE_LC709203
#include <Adafruit_LC709203F.h>
#endif

namespace TinyCon
{
enum class PowerSources : int8_t
{
    USB = 0,
    Battery = 1,
    I2C = 2
};

class PowerController
{
    static constexpr int8_t BatteryAdcPin = A6;
    static constexpr float BatteryPresentVoltage = 3.45f;
    static constexpr int8_t USBAdcPin = A7;
    static constexpr float USBPowerPresentVoltage = 4.6f;

public:
    explicit PowerController(TwoWire& i2c) : I2C(i2c) {}

    void Init();
    void Update();

    PowerSources PowerSource = PowerSources::USB;
    float USBPowerVoltage{};
    struct BatteryData
    {
        float Percentage;
        float Voltage;
        float Temperature;
    } Battery{};

private:
    TwoWire& I2C;

#if USE_LC709203
    bool LC709203FPresent = false;
    Adafruit_LC709203F LC709203F;
#endif
};
}