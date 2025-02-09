#pragma once

#include "Config.h"

#include "Utilities.h"
#include "GamepadController.h"

#include <cstdint>

class CommandProcessor
{
public:
    explicit CommandProcessor(GamepadController& controller) : Controller(controller) {}

    void ProcessCommand(Span command);

    uint8_t LastCommand{};
    uint8_t LastParameter{};
    uint8_t LastCommandStatus{};

    [[nodiscard]] bool GetI2CEnabled() const { return I2CEnabled; }
    [[nodiscard]] bool GetBLEEnabled() const { return BLEEnabled; }
    [[nodiscard]] bool GetUSBEnabled() const { return USBEnabled; }

    void SetI2CEnabled(bool enabled) { I2CEnabled = enabled; }
    void SetBLEEnabled(bool enabled) { BLEEnabled = enabled; }
    void SetUSBEnabled(bool enabled) { USBEnabled = enabled; }

private:
    GamepadController& Controller;
    bool I2CEnabled = true;
    bool USBEnabled = TINYCON_DEFAULT_USB_ENABLED;
    bool BLEEnabled = TINYCON_DEFAULT_BLE_ENABLED;
};
