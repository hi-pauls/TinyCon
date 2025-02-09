#include "GamepadController.h"

void GamepadController::Init()
{
    // We only support two controllers for now, if we need to support
    // more, might be worth using an I2C multiplexer instead.
    Haptics[1].Init(I2C0);
    Haptics[0].Init(I2C1);

    for (auto i = 0; i < MaxControllers; ++i)
    {
        Mpus[i].Init(I2C0, i);
        Inputs[i].Init(I2C0, i);
    }
}

void GamepadController::Update(uint32_t deltaTime)
{
    I2C0.setClock(1000000);
    LOG_CONTROLLER_LN("Controller Update:");
    for (int8_t i = 0; i < MaxControllers; ++i)
    {
        [[maybe_unused]] auto time = millis();
        Mpus[i].Update();
        LOG_CONTROLLER("    MPU "); LOG_CONTROLLER(i); LOG_CONTROLLER(": ("); LOG_CONTROLLER(Mpus[i].Acceleration.X); LOG_CONTROLLER(", "); LOG_CONTROLLER(Mpus[i].Acceleration.Y); LOG_CONTROLLER(", "); LOG_CONTROLLER(Mpus[i].Acceleration.Z);
        LOG_CONTROLLER("), ("); LOG_CONTROLLER(Mpus[i].AngularVelocity.X); LOG_CONTROLLER(", "); LOG_CONTROLLER(Mpus[i].AngularVelocity.Y); LOG_CONTROLLER(", "); LOG_CONTROLLER(Mpus[i].AngularVelocity.Z);
        LOG_CONTROLLER("), ("); LOG_CONTROLLER(Mpus[i].Orientation.X); LOG_CONTROLLER(", "); LOG_CONTROLLER(Mpus[i].Orientation.Y); LOG_CONTROLLER(", "); LOG_CONTROLLER(Mpus[i].Orientation.Z);
        LOG_CONTROLLER("), "); LOG_CONTROLLER(Mpus[i].Temperature);
        LOG_CONTROLLER(", "); LOG_CONTROLLER(millis() - time); LOG_CONTROLLER_LN("ms");

        time = millis();
        Inputs[i].Update();
        LOG_CONTROLLER("    Input "); LOG_CONTROLLER(i); LOG_CONTROLLER(": ("); LOG_CONTROLLER(Inputs[i].Axis[0]); LOG_CONTROLLER(", "); LOG_CONTROLLER(Inputs[i].Axis[1]); LOG_CONTROLLER("), ");
        LOG_CONTROLLER(Inputs[i].Buttons[0] ? "Down, " : "Up, "); LOG_CONTROLLER(Inputs[i].Buttons[1] ? "Down, " : "Up, "); LOG_CONTROLLER(Inputs[i].Buttons[2] ? "Down, " : "Up, "); LOG_CONTROLLER(Inputs[i].Buttons[3] ? "Down, " : "Up, "); LOG_CONTROLLER(Inputs[i].Buttons[4] ? "Down " : "Up ");
        LOG_CONTROLLER(", "); LOG_CONTROLLER(millis() - time); LOG_CONTROLLER_LN("ms");

        time = millis();
        LOG_CONTROLLER("    Haptic "); LOG_CONTROLLER(i); LOG_CONTROLLER(": "); LOG_CONTROLLER(Haptics[i].Available());
        Haptics[i].Update(deltaTime);
        LOG_CONTROLLER(", "); LOG_CONTROLLER(millis() - time); LOG_CONTROLLER_LN("ms");
    }

    I2C0.setClock(400000);
}

#if !NO_BLE || !NO_USB
hid_gamepad_report_t GamepadController::MakeHidReport() const
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

std::size_t GamepadController::MakeMpuBuffer(Span data) const
{
    auto size = 0;
    for (auto& mpu : Mpus)
        if (mpu.Present && mpu.Enabled)
            size += mpu.FillBuffer(data);
    return size;
}

void GamepadController::AddHapticCommand(Span data)
{
    uint8_t controller = data.Data[0];
    uint8_t command = data.Data[1];
    uint8_t count = data.Data[2];
    const uint8_t* sequence = data.Data + 3;
    uint16_t timeout = (data.Data[11] << 8) | data.Data[12];
    Haptics[controller].Insert(command, count, sequence, timeout);
    LOG_CONTROLLER("Add Haptic Command: "); LOG_CONTROLLER(controller); LOG_CONTROLLER(", "); LOG_CONTROLLER(command); LOG_CONTROLLER(", "); LOG_CONTROLLER(count); LOG_CONTROLLER(", "); LOG_CONTROLLER(timeout); LOG_CONTROLLER(", ");
}
