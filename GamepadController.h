#pragma once

#include "Config.h"

#include "HapticController.h"
#include "MpuController.h"
#include "InputController.h"

#include <Arduino.h>
#include <Wire.h>
#if !NO_BLE
#include <bluefruit.h>
#endif
#if !NO_USB
#include <Adafruit_TinyUSB.h>
#endif

class GamepadController
{
public:
    static constexpr uint8_t Version = 1;

    GamepadController(TwoWire& i2c0, SoftWire& i2c1) : I2C0(i2c0), I2C1(i2c1) {}

    void Init();
    void Update(uint32_t deltaTime);
#if !NO_BLE || !NO_USB
    [[nodiscard]] hid_gamepad_report_t MakeHidReport() const;
#endif
    [[nodiscard]] std::size_t MakeMpuBuffer(Span data) const;
    void AddHapticCommand(Span data);

    [[nodiscard]] MpuTypes GetMpuType(int8_t index) const { return Mpus[index].GetType(); }
    [[nodiscard]] HapticTypes GetHapticType(int8_t index) const { return Haptics[index].GetType(); }
    [[nodiscard]] ControllerTypes GetControllerType(int8_t index) const { return Inputs[index].GetType(); }
    [[nodiscard]] int8_t GetAxisCount(int8_t index) const { return Inputs[index].GetAxisCount(); }
    [[nodiscard]] int8_t GetButtonCount(int8_t index) const { return Inputs[index].GetButtonCount(); }
    [[nodiscard]] int8_t GetAxisCount() const { int8_t count = 0; for (auto& input : Inputs) count += input.GetAxisCount(); return count; }
    [[nodiscard]] int8_t GetButtonCount() const { int8_t count = 0; for (auto& input : Inputs) count += input.GetButtonCount(); return count; }

    [[nodiscard]] bool GetMpuPresent(int8_t index) const { return Mpus[index].Present; }
    [[nodiscard]] bool GetControllerEnabled(int8_t index) const { return Inputs[index].Enabled; }
    void SetControllerEnabled(int8_t index, bool enabled) { Inputs[index].Enabled = enabled; }
    [[nodiscard]] bool GetAccelerationEnabled(int8_t index) const { return Mpus[index].AccelerationEnabled; }
    void SetAccelerationEnabled(int8_t index, bool enabled) { Mpus[index].AccelerationEnabled = enabled; }
    [[nodiscard]] bool GetAngularVelocityEnabled(int8_t index) const { return Mpus[index].AngularVelocityEnabled; }
    void SetAngularVelocityEnabled(int8_t index, bool enabled) { Mpus[index].AngularVelocityEnabled = enabled; }
    [[nodiscard]] bool GetOrientationEnabled(int8_t index) const { return Mpus[index].OrientationEnabled; }
    void SetOrientationEnabled(int8_t index, bool enabled) { Mpus[index].OrientationEnabled = enabled; }
    [[nodiscard]] bool GetTemperatureEnabled(int8_t index) const { return Mpus[index].TemperatureEnabled; }
    void SetTemperatureEnabled(int8_t index, bool enabled) { Mpus[index].TemperatureEnabled = enabled; }
    [[nodiscard]] bool GetInputEnabled(int8_t index) const { return Inputs[index].Enabled; }
    void SetInputEnabled(int8_t index, bool enabled) { Inputs[index].Enabled = enabled; }
    [[nodiscard]] bool GetHapticEnabled(int8_t index) const { return Haptics[index].Enabled; }
    void SetHapticEnabled(int8_t index, bool enabled) { Haptics[index].Enabled = enabled; }

    [[nodiscard]] AccelerometerRanges GetAccelerometerRange() const { return Mpus[0].GetAccelerometerRange(); }
    void SetAccelerometerRange(AccelerometerRanges range) { for (auto& mpu : Mpus) mpu.SetAccelerometerRange(range); }
    [[nodiscard]] GyroscopeRanges GetGyroscopeRange() const { return Mpus[0].GetGyroscopeRange(); }
    void SetGyroscopeRange(GyroscopeRanges range) { for (auto& mpu : Mpus) mpu.SetGyroscopeRange(range); }

    [[nodiscard]] HapticController::HapticCommands GetHapticCommand(int8_t controller, int8_t commandIndex) const { return Haptics[controller].GetHapticCommand(commandIndex); }
    [[nodiscard]] uint8_t GetHapticCommandCount(int8_t controller, int8_t commandIndex) const { return Haptics[controller].GetHapticCommandCount(commandIndex); }
    [[nodiscard]] uint8_t GetHapticCommandData(int8_t controller, int8_t commandIndex, int8_t offset) const { return Haptics[controller].GetHapticCommandData(commandIndex, offset); }
    [[nodiscard]] uint16_t GetHapticCommandDuration(int8_t controller, int8_t commandIndex) const { return Haptics[controller].GetHapticCommandDuration(commandIndex); }
    [[nodiscard]] uint8_t GetHapticQueueSize(int8_t controller) const { return Haptics[controller].GetHapticQueueSize(); }
    void RemoveHapticCommand(int8_t controller, int8_t commandIndex) { Haptics[controller].RemoveHapticCommand(commandIndex); }
    void ClearHapticCommands() { for (auto& haptic : Haptics) haptic.ClearHapticCommands(); }

    [[nodiscard]] float GetAxis(int8_t controller, int8_t axisIndex)const  { return Inputs[controller].GetAxis(axisIndex); }
    [[nodiscard]] bool GetButton(int8_t controller, int8_t buttonIndex) const { return Inputs[controller].GetButton(buttonIndex); }
    [[nodiscard]] bool GetUpdatedButton(int8_t controller, int8_t buttonIndex) { return Inputs[controller].GetUpdatedButton(buttonIndex); }
    [[nodiscard]] Vector3 GetAcceleration(int8_t controller) const { return Mpus[controller].Acceleration; }
    [[nodiscard]] Vector3 GetAngularVelocity(int8_t controller) const { return Mpus[controller].AngularVelocity; }
    [[nodiscard]] Vector3 GetOrientation(int8_t controller) const { return Mpus[controller].Orientation; }

    static constexpr uint8_t MaxControllers = 2;
    uint8_t Id = 0;

private:
    TwoWire& I2C0;
    SoftWire& I2C1;
    HapticController Haptics[MaxControllers];
    MpuController Mpus[MaxControllers];
    InputController Inputs[MaxControllers];
};