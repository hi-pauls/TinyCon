#pragma once

#include "Config.h"

#include "Utilities.h"
#include "GamepadController.h"

#include "Core/Drivers/Input/TITinyConTypes.h"

#include <cstdint>

namespace TinyCon
{
class CommandProcessor
{
public:
    explicit CommandProcessor(GamepadController& controller) : Controller(controller) {}

    bool ProcessCommand(Tiny::Collections::TIFixedSpan<uint8_t> command);

    uint8_t LastCommand{};
    std::array<uint8_t, 2> LastParameter{};
    Tiny::Drivers::Input::TITinyConCommandStatus LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;

    [[nodiscard]] bool GetI2CEnabled() const { return I2CEnabled; }
    [[nodiscard]] bool GetBLEEnabled() const { return BLEEnabled; }
    [[nodiscard]] bool GetUSBEnabled() const { return USBEnabled; }

    void SetI2CEnabled(bool enabled) { I2CEnabled = enabled; }
    void SetBLEEnabled(bool enabled) { BLEEnabled = enabled; }
    void SetUSBEnabled(bool enabled) { USBEnabled = enabled; }

private:
    GamepadController& Controller;
    bool I2CEnabled = true;
    bool USBEnabled = TinyConUSBEnabledByDefault;
    bool BLEEnabled = TinyConBLEEnabledByDefault;
};
}