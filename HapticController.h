#pragma once

#include "Config.h"

#include "Utilities.h"
#include "Core/Drivers/Input/TITinyConTypes.h"

#include <Arduino.h>
#include <Wire.h>
#include <SoftWire.h>

namespace TinyCon
{
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

        void Init(TwoWire& wire);
        void Init(SoftWire& wire);
        void Insert(uint8_t command, uint8_t count, const uint8_t* data, uint16_t duration);
        void Update(int32_t deltaTime);

        [[nodiscard]] bool HasValues() const { return Tail != Head; }
        [[nodiscard]] int8_t Available() const { return (Head + MaxCommandCount - Tail) & (MaxCommandCount - 1); }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConHapticTypes GetType() const { return (DRV2605.Present && Enabled) ? Tiny::Drivers::Input::TITinyConHapticTypes::DRV2605 : Tiny::Drivers::Input::TITinyConHapticTypes::None; }
        [[nodiscard]] Tiny::Drivers::Input::TITinyConHapticCommands GetHapticCommand(int8_t index) const { return Commands[GetCommandIndex(index)].Command; }
        [[nodiscard]] uint8_t GetHapticCommandCount(int8_t index) const { return Commands[GetCommandIndex(index)].Count; }
        [[nodiscard]] uint8_t GetHapticCommandData(int8_t index, int8_t offset) const { return Commands[GetCommandIndex(index)].Value[offset]; }
        [[nodiscard]] uint16_t GetHapticCommandDuration(int8_t index) const { return Commands[GetCommandIndex(index)].Duration; }
        [[nodiscard]] uint8_t GetHapticQueueSize() const { return MaxCommandCount - Available() - 1; }
        void RemoveHapticCommand(int8_t index);
        void ClearHapticCommands() { Head = Tail = 0; }

        bool Enabled = true;
        bool Present = false;

        void Reset();
    private:
        DRV2605Controller DRV2605;

        struct
        {
            Tiny::Drivers::Input::TITinyConHapticCommands Command = Tiny::Drivers::Input::TITinyConHapticCommands::Noop;
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
}