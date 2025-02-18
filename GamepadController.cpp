#include "GamepadController.h"

using LogGamepad = Tiny::TILogTarget<TinyCon::GamepadLogLevel>;
using LogI2C = Tiny::TILogTarget<TinyCon::I2CLogLevel>;

    // more, might be worth using an I2C multiplexer instead.
    for (std::size_t i = 0; i < Inputs.size() - 1; ++i) Inputs[i].Init(I2C0, i);
    for (std::size_t i = 0; i < Mpus.size(); ++i) Mpus[i].Init(I2C0, i);
    Haptics[1].Init(I2C0);
    Haptics[0].Init(I2C1);
    HatOffset = hatOffset;
}

void TinyCon::GamepadController::Update(uint32_t deltaTime)
{
    if constexpr (Tiny::GlobalLogThreshold >= Tiny::TILogLevel::Verbose)
    {
        LogI2C::Verbose("I2C0 Devices: ");
    bool found = false;
        for (auto addr = 0x02; addr < 0x78; ++addr)
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
        for (auto addr = 0x02; addr < 0x78; ++addr)
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
    for (auto& mpu : Mpus)
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
    report.x = GetAxis(0) * 127;
    report.y = GetAxis(1) * 127;
    report.z = GetAxis(2) * 127;
    report.rz = GetAxis(3) * 127;
    report.rx = GetAxis(4) * 127;
    report.ry = GetAxis(5) * 127;

    if (HatOffset < 0) report.hat = GAMEPAD_HAT_CENTERED;
    else
    {
        // Figure out the hat, clock-wise starting at the top position button. Offsetting this by 5
        if (GetButton(HatOffset) && GetButton(HatOffset + 3)) report.hat = GAMEPAD_HAT_UP_LEFT;
        else if (GetButton(HatOffset) && GetButton(HatOffset + 1)) report.hat = GAMEPAD_HAT_UP_RIGHT;
        else if (GetButton(HatOffset)) report.hat = GAMEPAD_HAT_UP;
        else if (GetButton(HatOffset + 1) && GetButton(HatOffset + 2)) report.hat = GAMEPAD_HAT_DOWN_RIGHT;
        else if (GetButton(HatOffset + 1)) report.hat = GAMEPAD_HAT_RIGHT;
        else if (GetButton(HatOffset + 2) && GetButton(HatOffset + 3)) report.hat = GAMEPAD_HAT_DOWN_LEFT;
        else if (GetButton(HatOffset + 2)) report.hat = GAMEPAD_HAT_DOWN;
        else if (GetButton(HatOffset + 3)) report.hat = GAMEPAD_HAT_LEFT;
        else report.hat = GAMEPAD_HAT_CENTERED;
    }

    for (int8_t index = 0; index < 32; ++index)
        if (HatOffset < 0 || index < HatOffset) report.buttons |= GetButton(index) << (index);
        else report.buttons |= GetButton(index + 4) << (index);
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
float TinyCon::GamepadController::GetAxis(int8_t axisIndex) const
{
    for (auto& input : Inputs)
        if (axisIndex < input.GetAxisCount()) return input.Axis[axisIndex];
        else axisIndex -= input.GetAxisCount();
    return 0.0f;
}

bool TinyCon::GamepadController::GetButton(int8_t buttonIndex) const
{
    for (auto& input : Inputs)
        if (buttonIndex < input.GetButtonCount()) return input.Buttons[buttonIndex];
        else buttonIndex -= input.GetButtonCount();
    return false;
}
