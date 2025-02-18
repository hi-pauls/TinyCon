#pragma once

#include "Config.h"

#include "GamepadController.h"
#include "CommandProcessor.h"

#include <functional>

namespace TinyCon
{
#if NO_BLE
class BluetoothController
{
public:
    BluetoothController(GamepadController& controller, CommandProcessor& processor) {}
    void Init() {}
    void Update(uint32_t) {}
    void SetActive(bool) {}
    [[nodiscard]] bool IsConnected() const { return false; }
    [[nodiscard]] bool IsAdvertising() const { return false; }
};
#else
#include <bluefruit.h>
#include <services/BLEHIDGamepad.h>

void HapticWrite(uint16_t connection, BLECharacteristic* chr, uint8_t* data, uint16_t length);

class BluetoothController
{
public:
    BluetoothController(const GamepadController& controller, CommandProcessor& processor)
        : Controller(controller), Processor(processor) {}

    void Init();
    void Update(uint32_t deltaTime);
    void SetActive(bool active)
    {
        Serial.print(active ? "Bluetooth set Active" : "Bluetooth set Inactive"); Serial.println(Active ? " from Active" : " from Inactive");
        if (active && !Active) ForceAdvertise = true;
        Active = active;
    }

    [[nodiscard]] bool IsActive() const { return Active || Connected; }
    [[nodiscard]] bool IsAdvertising() const { return ForceAdvertise || AdvertisingTimeout > 0; }
    [[nodiscard]] bool IsConnected() const { return Connected; }

private:
    static std::function<void(uint16_t, BLECharacteristic*, uint8_t*, uint16_t)> HapticWriteCallback;
    static void HapticWrite(uint16_t connection, BLECharacteristic* chr, uint8_t* data, uint16_t length);

    bool Active = false;
    bool Connected = false;

    BLEDis Discovery;
    BLEHidGamepad GamepadService;
    BLEService MpuService = BLEService(0x181A);
    BLECharacteristic MpuCharacteristic = BLECharacteristic(0x2A58);
    BLEService HapticService = BLEService(0x1812);
    BLECharacteristic HapticCharacteristic = BLECharacteristic(0x2A4D);
    uint16_t ConnectionId = 0;

    constexpr static uint32_t AdvertisingTime = 40000;
    int32_t AdvertisingTimeout = 0;
    bool ForceAdvertise = false;
    bool AdvertisingStarted = false;

    const GamepadController& Controller;
    CommandProcessor& Processor;
};
#endif
}
