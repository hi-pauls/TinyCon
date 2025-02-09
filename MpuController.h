#pragma once

#include "Config.h"

#include "Utilities.h"

#include <Arduino.h>
#include <Adafruit_ICM20948.h>

enum class MpuTypes : uint8_t
{
    None = 0,
    ICM20948
};
constexpr uint8_t MpuTypeId(MpuTypes type) { return static_cast<uint8_t>(type); }

enum class AccelerometerRanges : uint8_t
{
    Invalid = 0,
    G2,
    G4,
    G8,
    G16
};
constexpr uint8_t AccelerometerRangeId(AccelerometerRanges range) { return static_cast<uint8_t>(range); }
constexpr AccelerometerRanges AccelerometerRange(uint8_t range) { return static_cast<AccelerometerRanges>(range); }

enum class GyroscopeRanges : uint8_t
{
    Invalid = 0,
    D250,
    D500,
    D1000,
    D2000,
    D4000
};
constexpr uint8_t GyroscopeRangeId(GyroscopeRanges range) { return static_cast<uint8_t>(range); }
constexpr GyroscopeRanges GyroscopeRange(uint8_t range) { return static_cast<GyroscopeRanges>(range); }

class MpuController
{
public:
    void Init(TwoWire& i2c, int8_t controller);
    void Update();
    [[nodiscard]] std::size_t FillBuffer(Span data) const;

    [[nodiscard]] MpuTypes GetType() const { return (Icm20948Present && Enabled) ? MpuTypes::ICM20948 : MpuTypes::None; }
    [[nodiscard]] AccelerometerRanges GetAccelerometerRange() const { return AccelerationRange; }
    void SetAccelerometerRange(AccelerometerRanges range);
    [[nodiscard]] GyroscopeRanges GetGyroscopeRange() const { return GyroscopeRange; }
    void SetGyroscopeRange(GyroscopeRanges range);

    bool Enabled = true;
    bool Present = false;
    bool AccelerationEnabled = true;
    bool AngularVelocityEnabled = true;
    bool OrientationEnabled = true;
    bool TemperatureEnabled = true;

    Vector3 Acceleration = {};
    Vector3 AngularVelocity = {};
    Vector3 Orientation = {};
    uint16_t Temperature = 0;

private:
    static constexpr int8_t ICM20948AddressByController[] = {0x69, 0x68};
    TwoWire* I2C;
    int8_t Controller;
    bool Icm20948Present = false;
    Adafruit_ICM20948 Icm20948;
    AccelerometerRanges AccelerationRange = AccelerometerRanges::G16;
    GyroscopeRanges GyroscopeRange = GyroscopeRanges::D2000;
};