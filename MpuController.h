#pragma once

#include "Config.h"
#include "Utilities.h"

#include "Core/Drivers/Input/TITinyConTypes.h"
#include "Core/Math/TIVector.h"
#include "Core/Utilities/Collections/TISpan.h"

#include <Arduino.h>
#include <Adafruit_ICM20948.h>

namespace TinyCon
{
    class MpuController
    {
    public:
        void Init(TwoWire& i2c, int8_t controller);
        void Update();
        [[nodiscard]] std::size_t FillBuffer(Tiny::Collections::TIFixedSpan<uint8_t> data) const;

        [[nodiscard]] Tiny::Drivers::Input::TITinyConMpuTypes GetType() const { return (Icm20948Present) ? Tiny::Drivers::Input::TITinyConMpuTypes::ICM20948 : Tiny::Drivers::Input::TITinyConMpuTypes::None; }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConAccelerometerRanges GetAccelerometerRange() const { return AccelerationRange; }
        void SetAccelerometerRange(Tiny::Drivers::Input::TITinyConAccelerometerRanges range);
        [[nodiscard]] Tiny::Drivers::Input::TITinyConGyroscopeRanges GetGyroscopeRange() const { return GyroscopeRange; }
        void SetGyroscopeRange(Tiny::Drivers::Input::TITinyConGyroscopeRanges range);

        bool Present = false;
        bool AccelerationEnabled = true;
        bool AngularVelocityEnabled = true;
        bool OrientationEnabled = true;
        bool TemperatureEnabled = true;

        Tiny::Math::TIVector3F Acceleration = {};
        Tiny::Math::TIVector3F AngularVelocity = {};
        Tiny::Math::TIVector3F Orientation = {};
        float Temperature = 0;

    private:
        static constexpr int8_t ICM20948AddressByController[] = {0x68, 0x69};
        static constexpr int8_t ICM20948AddressByControllerSize = sizeof(ICM20948AddressByController) / sizeof(ICM20948AddressByController[0]);
        TwoWire* I2C;
        int8_t Controller;
        bool Icm20948Present = false;
        Adafruit_ICM20948 Icm20948;
        Tiny::Drivers::Input::TITinyConAccelerometerRanges AccelerationRange = Tiny::Drivers::Input::TITinyConAccelerometerRanges::G16;
        Tiny::Drivers::Input::TITinyConGyroscopeRanges GyroscopeRange = Tiny::Drivers::Input::TITinyConGyroscopeRanges::D2000;
    };
}