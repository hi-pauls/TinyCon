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

namespace TinyCon
{
    class GamepadController
    {
    public:
        static constexpr uint8_t MaxInputControllers = 5;
        static constexpr uint8_t MaxMpuControllers = 2;
        static constexpr uint8_t MaxHapticControllers = 2;

        GamepadController(TwoWire& i2c0, SoftWire& i2c1) : I2C0(i2c0), I2C1(i2c1) {}

        void Init(int8_t hatOffset = -1, const std::array<int8_t, MaxNativeAdcPinCount>& axisPins = {NC}, const std::array<int8_t, MaxNativeGpioPinCount>& buttonPins = {NC}, ActiveState activeState = ActiveState::Low);
        void Update(uint32_t deltaTime);
    #if !NO_BLE || !NO_USB
        [[nodiscard]] hid_gamepad_report_t MakeHidReport() const;
    #endif
        [[nodiscard]] std::size_t MakeMpuBuffer(Tiny::Collections::TIFixedSpan<uint8_t> data) const;

        [[nodiscard]] bool GetInputPresent(int8_t controller) const { return Inputs[controller].Present; }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConControllerTypes GetControllerType(int8_t input) const { return Inputs[input].GetType(); }
        [[nodiscard]] int16_t GetAxisCount(int8_t input) const { return Inputs[input].GetAxisCount(); }
        [[nodiscard]] float GetAxis(int8_t controller, int8_t axisIndex)const  { return Inputs[controller].Axis[axisIndex]; }
        [[nodiscard]] int16_t GetAxisCount() const { int8_t count = 0; for (auto& input : Inputs) count += input.GetAxisCount(); return count; }
        [[nodiscard]] float GetAxis(int8_t axisIndex) const;
        [[nodiscard]] int16_t GetButtonCount(int8_t input) const { return Inputs[input].GetButtonCount(); }
        [[nodiscard]] bool GetButton(int8_t input, int8_t buttonIndex) const { return Inputs[input].Buttons[buttonIndex]; }
        [[nodiscard]] int16_t GetButtonCount() const { int8_t count = 0; for (auto& input : Inputs) count += input.GetButtonCount(); return count; }
        [[nodiscard]] bool GetButton(int8_t buttonIndex) const;
        [[nodiscard]] bool GetUpdatedButton(int8_t controller, int8_t buttonIndex) { return Inputs[controller].GetUpdatedButton(buttonIndex); }

        [[nodiscard]] bool GetAccelerationEnabled() const { return Mpus[0].AccelerationEnabled; }
        void SetAccelerationEnabled(bool enabled) { for (auto& mpu : Mpus) mpu.AccelerationEnabled = enabled; }
        [[nodiscard]] bool GetAngularVelocityEnabled() const { return Mpus[0].AngularVelocityEnabled; }
        void SetAngularVelocityEnabled(bool enabled) { for (auto& mpu : Mpus) mpu.AngularVelocityEnabled = enabled; }
        [[nodiscard]] bool GetOrientationEnabled() const { return Mpus[0].OrientationEnabled; }
        void SetOrientationEnabled(bool enabled) { for (auto& mpu : Mpus) mpu.OrientationEnabled = enabled; }
        [[nodiscard]] bool GetTemperatureEnabled() const { return Mpus[0].TemperatureEnabled; }
        void SetTemperatureEnabled(bool enabled) { for (auto& mpu : Mpus) mpu.TemperatureEnabled = enabled; }
        [[nodiscard]] int8_t GetMpuCount() const { int8_t count = 0; for (auto & Mpu : Mpus) if (Mpu.Present) ++count; return count; }
        [[nodiscard]] bool GetMpuPresent(int8_t mpu) const { return Mpus[mpu].Present; }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConMpuTypes GetMpuType(int8_t mpu) const { return Mpus[mpu].GetType(); }

        [[nodiscard]] Tiny::Drivers::Input::TITinyConAccelerometerRanges GetAccelerometerRange(int8_t mpu) const { return Mpus[mpu].GetAccelerometerRange(); }
        void SetAccelerometerRange(int8_t mpu, Tiny::Drivers::Input::TITinyConAccelerometerRanges range) { Mpus[mpu].SetAccelerometerRange(range); }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConGyroscopeRanges GetGyroscopeRange(int8_t mpu) const { return Mpus[mpu].GetGyroscopeRange(); }
        void SetGyroscopeRange(int8_t mpu, Tiny::Drivers::Input::TITinyConGyroscopeRanges range) { Mpus[mpu].SetGyroscopeRange(range); }

        [[nodiscard]] Tiny::Math::TIVector3F GetAcceleration(int8_t mpu) const { return Mpus[mpu].Acceleration; }
        [[nodiscard]] Tiny::Math::TIVector3F GetAngularVelocity(int8_t mpu) const { return Mpus[mpu].AngularVelocity; }
        [[nodiscard]] Tiny::Math::TIVector3F GetOrientation(int8_t mpu) const { return Mpus[mpu].Orientation; }

        [[nodiscard]] bool GetHapticEnabled(int8_t haptic) const { return Haptics[haptic].Enabled; }
        void SetHapticEnabled(int8_t haptic, bool enabled) { Haptics[haptic].Enabled = enabled; }
        [[nodiscard]] bool GetHapticPresent(int8_t haptic) const { return Haptics[haptic].Present; }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConHapticTypes GetHapticType(int8_t haptic) const { return Haptics[haptic].GetType(); }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConHapticCommands GetHapticCommand(int8_t haptic, int8_t commandIndex) const { return Haptics[haptic].GetHapticCommand(commandIndex); }
        [[nodiscard]] uint8_t GetHapticCommandCount(int8_t haptic, int8_t commandIndex) const { return Haptics[haptic].GetHapticCommandCount(commandIndex); }
        [[nodiscard]] uint8_t GetHapticCommandData(int8_t haptic, int8_t commandIndex, int8_t offset) const { return Haptics[haptic].GetHapticCommandData(commandIndex, offset); }
        [[nodiscard]] uint16_t GetHapticCommandDuration(int8_t haptic, int8_t commandIndex) const { return Haptics[haptic].GetHapticCommandDuration(commandIndex); }
        [[nodiscard]] uint8_t GetHapticQueueSize(int8_t haptic) const { return Haptics[haptic].GetHapticQueueSize(); }
        void RemoveHapticCommand(int8_t haptic, int8_t commandIndex) { Haptics[haptic].RemoveHapticCommand(commandIndex); }
        void ClearHapticCommands() { for (auto& haptic : Haptics) haptic.ClearHapticCommands(); }
        void AddHapticCommand(Tiny::Collections::TIFixedSpan<uint8_t> data);

        uint8_t Id = 0;

        void Reset();

    private:
        TwoWire& I2C0;
        SoftWire& I2C1;
        std::array<HapticController, MaxHapticControllers> Haptics{};
        std::array<MpuController, MaxMpuControllers> Mpus{};
        std::array<InputController, MaxInputControllers> Inputs{};
        int8_t HatOffset = -1;
    };
}
