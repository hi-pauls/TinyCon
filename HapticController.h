#pragma once

#include "Config.h"

#include "Utilities.h"

#include <Arduino.h>
#include <Wire.h>
#include <SoftWire.h>

enum class HapticTypes : uint8_t
{
    None = 0,
    DRV2605
};
constexpr uint8_t HapticTypeId(HapticTypes type) { return static_cast<uint8_t>(type); }

class DRV2605Controller
{
public:
    static constexpr uint8_t DRV2605_MODE_INTTRIG = 0x00;
    static constexpr uint8_t DRV2605_MODE_REALTIME = 0x05;

    void Init(TwoWire& wire);
    void Init(SoftWire& wire);
    void PlayRealtime(uint8_t value);
    void PlayWaveform(const uint8_t* data);
    void Stop();

    bool Present = false;

private:
    uint8_t Mode = DRV2605_MODE_INTTRIG;
    union { TwoWire* Hardware; SoftWire* Software; } I2C;
    bool SoftwareMode = false;

    void Init();
};

class HapticController
{
public:
    static constexpr int8_t MaxCommandCount = 8;

    enum class HapticCommands
    {
        Noop = 0,
        PlayWaveform = 0x1,
        PlayRealtime = 0x2
    };
    static constexpr uint8_t HapticCommandId(HapticCommands command) { return static_cast<uint8_t>(command); }

    void Init(TwoWire& wire);
    void Init(SoftWire& wire);
    void Insert(uint8_t command, uint8_t count, const uint8_t* data, uint16_t duration);
    void Update(int32_t deltaTime);

    [[nodiscard]] bool HasValues() const { return Tail != Head; }
    [[nodiscard]] int8_t Available() const { return (Head + MaxCommandCount - Tail) & (MaxCommandCount - 1); }
    [[nodiscard]] HapticTypes GetType() const { return (DRV2605.Present && Enabled) ? HapticTypes::DRV2605 : HapticTypes::None; }
    [[nodiscard]] HapticCommands GetHapticCommand(int8_t index) const { return Commands[GetCommandIndex(index)].Command; }
    [[nodiscard]] uint8_t GetHapticCommandCount(int8_t index) const { return Commands[GetCommandIndex(index)].Count; }
    [[nodiscard]] uint8_t GetHapticCommandData(int8_t index, int8_t offset) const { return Commands[GetCommandIndex(index)].Value[offset]; }
    [[nodiscard]] uint16_t GetHapticCommandDuration(int8_t index) const { return Commands[GetCommandIndex(index)].Duration; }
    [[nodiscard]] uint8_t GetHapticQueueSize() const { return MaxCommandCount - Available() - 1; }
    void RemoveHapticCommand(int8_t index);
    void ClearHapticCommands() { Head = Tail = 0; }

    bool Enabled = true;
    bool Present = false;

private:
    DRV2605Controller DRV2605;

    struct
    {
        HapticCommands Command = HapticCommands::Noop;
        uint8_t Count = 0;
        uint8_t Value[8] = {};
        uint16_t Duration = 0;
    } Commands[MaxCommandCount] = {};
    int8_t Head = 0;
    int8_t Tail = 0;

    int8_t RealtimeIndex = 0;
    int16_t RealtimeTimeLeft = 0;
    int16_t RealtimePerCommandDuration = 0;

    [[nodiscard]] int8_t GetCommandIndex(int8_t index) const { return (Tail + index) & 7; }
    bool HasNewCommand(int32_t deltaTime);
};
