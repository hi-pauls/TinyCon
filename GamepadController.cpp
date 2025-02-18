#include "GamepadController.h"

using LogGamepad = Tiny::TILogTarget<TinyCon::GamepadLogLevel>;
using LogI2C = Tiny::TILogTarget<TinyCon::I2CLogLevel>;

    // more, might be worth using an I2C multiplexer instead.
    Haptics[1].Init(I2C0);
    Haptics[0].Init(I2C1);

    for (auto i = 0; i < MaxControllers; ++i)
    {
        Mpus[i].Init(I2C0, i);
        Inputs[i].Init(I2C0, i);
    }
}

void TinyCon::GamepadController::Update(uint32_t deltaTime)
{
    if constexpr (Tiny::GlobalLogThreshold >= Tiny::TILogLevel::Verbose)
    {
        LogI2C::Verbose("I2C0 Devices: ");
    bool found = false;
    for (auto addr = 0x02; addr < 0x70; ++addr)
    {
        I2C0.beginTransmission(addr);
        if (I2C0.endTransmission() == 0)
        {
                if (found) LogI2C::Verbose(", ");
            found = true;
                LogI2C::Verbose("0x", addr, Tiny::TIFormat::Hex);
        }
    }

        LogI2C::Verbose(Tiny::TIEndl);
        LogI2C::Verbose("I2C1 Devices: ");
    found = false;
    for (auto addr = 0x08; addr < 0x70; ++addr)
    {
        I2C1.beginTransmission(addr);
        if (I2C1.endTransmission() == 0)
        {
                if (found) LogI2C::Verbose(", ");
            found = true;
                LogI2C::Verbose("0x", addr, Tiny::TIFormat::Hex);
            }
        }
        LogI2C::Verbose(Tiny::TIEndl);
    }
#endif

    I2C0.setClock(1000000);
    LogGamepad::Info("Controller Update:");
    for (int8_t i = 0; i < MaxControllers; ++i)
    {
        [[maybe_unused]] auto time = millis();
        Mpus[i].Update();
            LogGamepad::Debug("    MPU: (", mpu.Acceleration.X, ", ", mpu.Acceleration.Y, ", ", mpu.Acceleration.Z,
                              "), (", mpu.AngularVelocity.X, ", ", mpu.AngularVelocity.Y, ", ", mpu.AngularVelocity.Z,
                              "), (", mpu.Orientation.X, ", ", mpu.Orientation.Y, ", ", mpu.Orientation.Z,
                              "), ", mpu.Temperature, ", ", millis() - time, "ms", Tiny::TIEndl);
        }

    for (auto& input : Inputs)
        Inputs[i].Update();
        {
            auto time = millis();
            LogGamepad::Debug("    Input: (");
            input.Update();
        time = millis();
            {
        Haptics[i].Update(deltaTime);
                LogGamepad::Debug(input.Axis[j]);
            LogGamepad::Debug("), (");
            for (int8_t j = 0; j < input.GetButtonCount(); ++j)
            {
                if (j > 0) LogGamepad::Debug(", ");
                LogGamepad::Debug(input.Buttons[j] ? "Down" : "Up");
            }
            LogGamepad::Debug("), ", millis() - time, "ms", Tiny::TIEndl);
    }

            LogGamepad::Debug("    Haptic: ", haptic.Available());
            LogGamepad::Debug(", ", millis() - time, "ms", Tiny::TIEndl);
    I2C0.setClock(400000);
}

#if !NO_BLE || !NO_USB
hid_gamepad_report_t TinyCon::GamepadController::MakeHidReport() const
{
    hid_gamepad_report_t report = {};

    // XXX: Figure out how we can support more than 2+1 axis per controller
    // Assume any extra axis is 0-ed if not available or the controller is disabled
    report.x = Inputs[0].Axis[0] * 127;
    report.y = Inputs[0].Axis[1] * 127;
    report.z = Inputs[1].Axis[0] * 127;
    report.rz = Inputs[1].Axis[1] * 127;
    report.rx = Inputs[0].Axis[2] * 127;
    report.ry = Inputs[1].Axis[2] * 127;

    // Figure out the head, clock-wise starting at the top position button
    if (Inputs[1].Buttons[0] && Inputs[1].Buttons[3]) report.hat = GAMEPAD_HAT_UP_LEFT;
    else if (Inputs[1].Buttons[0] && Inputs[1].Buttons[1]) report.hat = GAMEPAD_HAT_UP_RIGHT;
    else if (Inputs[1].Buttons[0]) report.hat = GAMEPAD_HAT_UP;
    else if (Inputs[1].Buttons[1] && Inputs[1].Buttons[2]) report.hat = GAMEPAD_HAT_DOWN_RIGHT;
    else if (Inputs[1].Buttons[1]) report.hat = GAMEPAD_HAT_RIGHT;
    else if (Inputs[1].Buttons[2] && Inputs[1].Buttons[3]) report.hat = GAMEPAD_HAT_DOWN_LEFT;
    else if (Inputs[1].Buttons[2]) report.hat = GAMEPAD_HAT_DOWN;
    else if (Inputs[1].Buttons[3]) report.hat = GAMEPAD_HAT_LEFT;

    // XXX: Figure out how to support more than 16 buttons per controller
    // Assume any extra buttons are 0-ed if not available or the controller is disabled
    // We are offsetting the report of the right buttons by 4 because they are already accounted for with the head.
    for (int8_t index = 0; index < 16; ++index)
        if (index < 12) report.buttons |= (Inputs[0].Buttons[index] << index) | (Inputs[1].Buttons[index + 4] << (index + 16));
        else report.buttons |= Inputs[0].Buttons[index] << index;
    return report;
}
#endif

std::size_t TinyCon::GamepadController::MakeMpuBuffer(Tiny::Collections::TIFixedSpan<uint8_t> data) const
{
    auto size = 0;
    for (auto& mpu : Mpus)
        if (mpu.Present && mpu.Enabled)
            size += mpu.FillBuffer(data);
    return size;
}

void TinyCon::GamepadController::AddHapticCommand(Tiny::Collections::TIFixedSpan<uint8_t> data)
{
    uint8_t controller = data.Data[0];
    uint8_t command = data.Data[1];
    uint8_t count = data.Data[2];
    const uint8_t* sequence = data.Data + 3;
    uint16_t timeout = (data.Data[11] << 8) | data.Data[12];
    Haptics[controller].Insert(command, count, sequence, timeout);
            LogGamepad::Info("Add Haptic Command: ", controller, ", ", command, ", ", count, ", ", timeout, Tiny::TIEndl);
}
