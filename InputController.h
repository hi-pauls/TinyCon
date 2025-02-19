#pragma once

#include "Config.h"

#include "Utilities.h"

#include <Arduino.h>
#include <Adafruit_seesaw.h>
#include <array>

namespace TinyCon
{
    enum class ControllerTypes : uint8_t
    {
        None = 0,
        Seesaw,
        Pins
    };
    constexpr uint8_t ControllerTypeId(ControllerTypes type) { return static_cast<uint8_t>(type); }

    struct DebouncedButton
    {
        bool History[4] = {};
        int8_t Head = 0;

        void AddState(bool state) { History[Head] = state; Head = (Head + 1) & 3; }
        [[nodiscard]] bool StateAt(int8_t index) const { index = (4 + Head - index) & 3; return History[index]; }
        [[nodiscard]] bool Get() const;
    };

    class SeesawController
    {
    public:
        void Init(TwoWire& i2c, int8_t controller);
        void Update();

        float Axis[2] = {};
        DebouncedButton Buttons[5] = {};
        bool Present = false;

        // Special call to just update a single special button
        bool GetUpdatedButton(int8_t index);

    private:
        Adafruit_seesaw Device;
        int8_t Controller = -1;
        uint8_t EncoderPosition = 0;

        static constexpr int8_t AddressByController[] = {0x49, 0x4A, 0x4B, 0x4C};
        static constexpr uint8_t InputAxis[] = {2, 3};
        static constexpr int32_t InputAxisCount = sizeof(InputAxis) / sizeof(InputAxis[0]);
        static constexpr uint8_t InputButtonRight = 6;
        static constexpr uint8_t InputButtonDown = 7;
        static constexpr uint8_t InputButtonLeft = 9;
        static constexpr uint8_t InputButtonUp = 10;
        static constexpr uint8_t InputButtonSelect = 14;
        static constexpr uint32_t InputButtons[] = {1 << InputButtonRight, 1 << InputButtonDown, 1 << InputButtonLeft, 1 << InputButtonUp, 1 << InputButtonSelect};
        static constexpr uint32_t InputButtonMask = InputButtons[0] | InputButtons[1] | InputButtons[2] | InputButtons[3] | InputButtons[4];
        static constexpr int32_t InputButtonCount = sizeof(InputButtons) / sizeof(InputButtons[0]);

        void Init();
    };

    class PinsInputController
    {
        template <typename TArray>
        static constexpr int16_t CountNotNC(const TArray& pins)
        {
            for (std::size_t count = 0; count < pins.size(); ++count) if (pins[count] == NC) return static_cast<int16_t>(count);
            return pins.size();
        }
    public:
        void Init(const std::array<int8_t, MaxNativeAdcPinCount>& axisPins, const std::array<int8_t, MaxNativeGpioPinCount>& buttonPins, ActiveState activeState);
        void Update();

        float Axis[2] = {};
        DebouncedButton Buttons[5] = {};
        bool Present = false;

        // Special call to just update a single special button
        bool GetUpdatedButton(int8_t index);

        [[nodiscard]] int16_t GetAxisCount() const { return AxisCount; }
        [[nodiscard]] int16_t GetButtonCount() const { return ButtonCount; }
    private:
        std::array<int8_t, 2> AxisPins = {NC};
        int16_t AxisCount = 0;
        std::array<int8_t, 5> ButtonPins = {NC};
        int16_t ButtonCount = 0;
        ActiveState ButtonActiveState = ActiveState::Low;
    };

    class InputController
    {
    public:
        InputController() {};
        ~InputController() { if (Type == ControllerTypes::Seesaw) Seesaw.~SeesawController(); }

        void Init(TwoWire& i2c, int8_t controller);
        void Init(const std::array<int8_t, MaxNativeAdcPinCount>& axisPins, const std::array<int8_t, MaxNativeGpioPinCount>& buttonPins, ActiveState activeState);
        void Update();
        [[nodiscard]] ControllerTypes GetType() const;
        [[nodiscard]] int16_t GetAxisCount() const;
        [[nodiscard]] int16_t GetButtonCount() const;

        [[nodiscard]] float GetAxis(int8_t index) const { return Axis[index]; }
        [[nodiscard]] bool GetButton(int8_t index) const { return Buttons[index]; }
        bool GetUpdatedButton(int8_t index);

        bool Enabled = true;
        bool Present = false;
        float Axis[8] = {};
        bool Buttons[32] = {};

    private:
        ControllerTypes Type = ControllerTypes::None;
        union
        {
            PinsInputController Pins;
            SeesawController Seesaw{};
        };
    };
}