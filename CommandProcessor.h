#pragma once

#include "Config.h"

#include "GamepadController.h"
#include "Power.h"
#include "Utilities.h"

#include "Core/Drivers/Input/TITinyConTypes.h"

#include <cstdint>

namespace TinyCon
{
    class CommandProcessor
    {
        static constexpr int16_t MaxRegisters = 0x42 + GamepadController::MaxMpuControllers * 20 + GamepadController::MaxInputControllers * (8 * 2 + 4);

    public:
        constexpr static int8_t MaxCommandSize = 14;

        explicit CommandProcessor(GamepadController& controller, const PowerController& power)
            : Controller(controller), Power(power) {}

        void Init();
        void Update();
        bool ProcessCommand(Tiny::Collections::TIFixedSpan<uint8_t> command);

        [[nodiscard]] bool GetI2CEnabled() const { return I2CEnabled; }
        void SetI2CEnabled(bool enabled) { I2CEnabled = enabled; }
        [[nodiscard]] bool GetBLEEnabled() const { return BLEEnabled; }
        void SetBLEEnabled(bool enabled) { BLEEnabled = enabled; }
        [[nodiscard]] bool GetUSBEnabled() const { return USBEnabled; }
        void SetUSBEnabled(bool enabled) { USBEnabled = enabled; }

        std::array<uint8_t, MaxRegisters> Registers = {};

        uint8_t LastCommand = 0;
        std::array<uint8_t, 2> LastParameter = {};
        Tiny::Drivers::Input::TITinyConCommandStatus LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;

    private:
        GamepadController& Controller;
        const PowerController& Power;

        bool I2CEnabled = true;
        bool USBEnabled = TinyConUSBEnabledByDefault;
        bool BLEEnabled = TinyConBLEEnabledByDefault;

        /**
         * Using a buffer here instead of just-in-time generation because we are using larger 32-bit MCUs
         * with enough memory available, and because it is much faster to fill the TX buffer in the ISR.
         */
        void SetRegister(Tiny::Drivers::Input::TITinyConCommands command, uint8_t offset, uint8_t value) { Registers[static_cast<uint8_t>(command) + offset] = value; }
        void SetRegister(Tiny::Drivers::Input::TITinyConCommands command, uint8_t value) { SetRegister(command, 0, value); }
    };
}