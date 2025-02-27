#include "MpuController.h"

void TinyCon::MpuController::Init(TwoWire& i2c, int8_t controller)
{
    I2C = &i2c;
    Controller = controller;
}

void TinyCon::MpuController::Update()
{
    if (!Icm20948Present)
    {
        if (Controller < ICM20948AddressByControllerSize)
        {
            const auto address = ICM20948AddressByController[Controller];
            I2C->beginTransmission(address);
            if (I2C->endTransmission() == 0 && (Icm20948Present = Icm20948.begin_I2C(address, I2C)))
            {
                delay(100);
                SetAccelerometerRange(AccelerationRange);
                SetGyroscopeRange(GyroscopeRange);
                Icm20948.setMagDataRate(AK09916_MAG_DATARATE_50_HZ);
                Present = true;
                delay(10);
            }
        }
    }
    else
    {
        // Grab latest MPU values
        sensors_event_t accelerationEvent, angularVelocityEvent, orientationEvent, temperatureEvent;
        Icm20948.getEvent(&accelerationEvent, &angularVelocityEvent, &temperatureEvent, &orientationEvent);
        Acceleration = {accelerationEvent.acceleration.x, accelerationEvent.acceleration.y, accelerationEvent.acceleration.z};
        AngularVelocity = {angularVelocityEvent.gyro.x, angularVelocityEvent.gyro.y, angularVelocityEvent.gyro.z};
        Orientation = {orientationEvent.orientation.x, orientationEvent.orientation.y, orientationEvent.orientation.z};
        Temperature = temperatureEvent.temperature;
    }
}

std::size_t TinyCon::MpuController::FillBuffer(Tiny::Collections::TIFixedSpan<uint8_t> data) const
{
    auto size = 0;
    auto* current = const_cast<uint8_t*>(data.data());
    if (AccelerationEnabled)
    {
        FillHalf(current, Acceleration.X);
        FillHalf(current, Acceleration.Y);
        FillHalf(current, Acceleration.Z);
        size += 6;
    }
    if (AngularVelocityEnabled)
    {
        FillHalf(current, AngularVelocity.X);
        FillHalf(current, AngularVelocity.Y);
        FillHalf(current, AngularVelocity.Z);
        size += 6;
    }
    if (OrientationEnabled)
    {
        FillHalf(current , Orientation.X);
        FillHalf(current , Orientation.Y);
        FillHalf(current , Orientation.Z);
        size += 6;
    }
    if (TemperatureEnabled)
    {
        FillHalf(current , Temperature);
        size += 2;
    }

    return size;
}

void TinyCon::MpuController::SetAccelerometerRange(Tiny::Drivers::Input::TITinyConAccelerometerRanges range)
{
    AccelerationRange = range;
    if (Icm20948Present)
    {
        switch (range)
        {
            case Tiny::Drivers::Input::TITinyConAccelerometerRanges::G2: Icm20948.setAccelRange(ICM20948_ACCEL_RANGE_2_G); break;
            case Tiny::Drivers::Input::TITinyConAccelerometerRanges::G4: Icm20948.setAccelRange(ICM20948_ACCEL_RANGE_4_G); break;
            case Tiny::Drivers::Input::TITinyConAccelerometerRanges::G8: Icm20948.setAccelRange(ICM20948_ACCEL_RANGE_8_G); break;
            default: Icm20948.setAccelRange(ICM20948_ACCEL_RANGE_16_G); break;
        }
    }
}

void TinyCon::MpuController::SetGyroscopeRange(Tiny::Drivers::Input::TITinyConGyroscopeRanges range)
{
    GyroscopeRange = range;
    if (Icm20948Present)
    {
        switch (range)
        {
            case Tiny::Drivers::Input::TITinyConGyroscopeRanges::D250: Icm20948.setGyroRange(ICM20948_GYRO_RANGE_250_DPS); break;
            case Tiny::Drivers::Input::TITinyConGyroscopeRanges::D500: Icm20948.setGyroRange(ICM20948_GYRO_RANGE_500_DPS); break;
            case Tiny::Drivers::Input::TITinyConGyroscopeRanges::D1000: Icm20948.setGyroRange(ICM20948_GYRO_RANGE_1000_DPS); break;
            default: Icm20948.setGyroRange(ICM20948_GYRO_RANGE_2000_DPS); break;
        }
    }
}

void TinyCon::MpuController::Reset()
{
    Present = Icm20948Present = false;
    Acceleration = {0, 0, 0};
    AngularVelocity = {0, 0, 0};
    Orientation = {0, 0, 0};
    Temperature = 0;
    AccelerationEnabled = true;
    AngularVelocityEnabled = true;
    OrientationEnabled = true;
    TemperatureEnabled = true;
    AccelerationRange = Tiny::Drivers::Input::TITinyConAccelerometerRanges::G16;
    GyroscopeRange = Tiny::Drivers::Input::TITinyConGyroscopeRanges::D2000;
}
