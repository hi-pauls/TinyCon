#pragma once

#include "Config.h"

#include "Bluetooth.h"
#include "CommandProcessor.h"
#include "GamepadController.h"
#include "I2C.h"
#include "Indicators.h"
#include "Power.h"
#include "USB.h"

#include <Arduino.h>
#include <Wire.h>
#include <SoftWire.h>

namespace TinyCon
{
    class TinyController
    {
        static constexpr auto BluetoothStartButtonIndex = 4;
        static constexpr auto BluetoothStartButtonTime = 5 * 1000;

    public:
        TinyController(TwoWire& slaveI2C, TwoWire& masterI2C0, SoftWire& masterI2C1)
            : Controller(masterI2C0, masterI2C1), Power(masterI2C0), Processor(Controller),
              USBControl(Controller, Processor), Bluetooth(Controller, Processor),
              Indicators(masterI2C0, Controller, Power), I2C(slaveI2C, Controller, Power, Processor) {}

        void Init(int8_t hatOffset = -1, const std::array<int8_t, MaxNativeAdcPinCount>& axisPins = {NC}, const std::array<int8_t, MaxNativeGpioPinCount>& buttonPins = {NC}, ActiveState activeState = ActiveState::Low);
        void Update(int32_t deltaTime);

        void AddHapticCommand(Tiny::Collections::TIFixedSpan<uint8_t> data) { Controller.AddHapticCommand(data); }

        [[nodiscard]] bool IsSuspended() const { return Suspended; }

    private:
        GamepadController Controller;
        PowerController Power;
        CommandProcessor Processor;
        USBController USBControl;
        BluetoothController Bluetooth;
        IndicatorController Indicators;
        I2CController I2C;

        int32_t BluetoothStartPressedTimeout = BluetoothStartButtonTime;
        bool Suspended = false;

        void UpdateIndicators(int32_t deltaTime);

        void UpdateSelectButton(int32_t deltaTime, bool selectButton);
    };
}
