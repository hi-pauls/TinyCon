#pragma once

#include "Config.h"

#include "CommandProcessor.h"
#include "GamepadController.h"
#include "Power.h"
#include "Utilities.h"

#include <Arduino.h>
#include <Wire.h>
#include <functional>

namespace TinyCon
{
    class I2CController
    {
    public:
        I2CController(TwoWire& slaveI2C, const GamepadController& controller, const PowerController& power, CommandProcessor& processor)
            : SlaveI2C(slaveI2C), Controller(controller), Power(power), Processor(processor) {}

        void Init();
        void Update();
        void Send();
        void Receive();

    private:
        static void I2CSlaveReceive(int count);
        static void I2CSlaveRequest();
        static std::function<void(int)> I2CReceiveCallback;
        static std::function<void()> I2CRequestCallback;

        TwoWire& SlaveI2C;
        const GamepadController& Controller;
        const PowerController& Power;
        CommandProcessor& Processor;

        uint8_t RegisterAddress = 0;
        uint32_t Timeout = 200;
        uint32_t LastInputTime = 0;
        std::array<uint8_t, 14> Buffer = {};
        uint8_t BufferIndex = 0;

        static constexpr int16_t MaxI2CRegisters = 0x40 + GamepadController::MaxMpuControllers * 20 + GamepadController::MaxInputControllers * (8 * 2 + 4);
        /**
         * Using a buffer here instead of just-in-time generation because we are using larger 32-bit MCUs
         * with enough memory available, and because it is much faster to fill the TX buffer in the ISR.
         */
        std::array<uint8_t, MaxI2CRegisters> Registers = {};
        void SetRegister(Tiny::Drivers::Input::TITinyConCommands command, uint8_t offset, uint8_t value)
        {
            Registers[Tiny::Drivers::Input::TITinyConCommandId(command) + offset] = value;
        }

        void SetRegister(Tiny::Drivers::Input::TITinyConCommands command, uint8_t value) { SetRegister(command, 0, value); }
    };
}