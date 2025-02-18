#include "InputController.h"

bool TinyCon::DebouncedButton::Get() const
{
    int8_t count = 0;
    for (int8_t i = 0; i < 4; ++i)
        if (StateAt(i) && ++count > 2) return true;
    return false;
}

void TinyCon::SeesawController::Init(TwoWire& i2c, int8_t controller)
{
    Device = {&i2c};
    Controller = controller;
    if ((Present = Device.begin(AddressByController[Controller]))) Init();
}

void TinyCon::SeesawController::Init()
{
    Device.pinModeBulk(InputButtonMask, INPUT_PULLUP);
    Device.setGPIOInterrupts(InputButtonMask, 1);
}

void TinyCon::SeesawController::Update()
{
    if (!Present) return;

    // Get latest input values
    auto buttonStates = Device.digitalReadBulk(InputButtonMask);
    if (buttonStates == 0)
    {
        // This is relevant for Seesaw inputs we connect via Stemma QT, since they may still have a reset button.
        // This will result in the input register being all 0's, looking like all buttons are pressed at the same
        // time. Try to reinitialize the device, causing a software reset in the process. Discard the input after,
        // so we don't get random results because of the cached button states.
        Device.SWReset();
        Init();
        return;
    }

    for (auto i = 0; i < InputButtonCount; ++i)
        Buttons[i].AddState((buttonStates & InputButtons[i]) == 0);
    for (auto i = 0; i < InputAxisCount; ++i)
        Axis[i] = Device.analogRead(InputAxis[i]) / 512.0f - 1.0f;
}

bool TinyCon::SeesawController::GetUpdatedButton(int8_t index)
{
    const auto mask = InputButtons[index];
    return Device.digitalReadBulk(mask) == 0;
}

void TinyCon::PinsInputController::Update()
{
    for (std::size_t axisIndex = 0; axisIndex < AxisPins.size(); ++axisIndex)
        Axis[axisIndex] = analogRead(AxisPins[axisIndex]) / 512.0f - 1.0f;
    for (std::size_t buttonIndex = 0; buttonIndex < ButtonPins.size(); ++buttonIndex)
        Buttons[buttonIndex].AddState(digitalRead(ButtonPins[buttonIndex]) == ((ButtonActiveState == ActiveState::High) ? HIGH : LOW));
}

bool TinyCon::PinsInputController::GetUpdatedButton(int8_t index)
{
    return digitalRead(ButtonPins[index]) == ((ButtonActiveState == ActiveState::High) ? HIGH : LOW);
}

void TinyCon::PinsInputController::Init(const std::array<int8_t, MaxNativeAdcPinCount>& axisPins, const std::array<int8_t, MaxNativeGpioPinCount>& buttonPins, ActiveState activeState)
{
    AxisCount = CountNotNC(axisPins);
    ButtonCount = CountNotNC(buttonPins);
    Present = AxisCount > 0 || ButtonCount > 0;
    ButtonActiveState = activeState;
    AxisPins = axisPins;
    ButtonPins = buttonPins;
}


void TinyCon::InputController::Init(const std::array<int8_t, MaxNativeAdcPinCount>& axisPins, const std::array<int8_t, MaxNativeGpioPinCount>& buttonPins, ActiveState activeState)
{
    Pins.Init(axisPins, buttonPins, activeState);
    if (Pins.Present)
    {
        Type = ControllerTypes::Pins;
        Present = true;
    }
    else Present = false;
}

void TinyCon::InputController::Init(TwoWire& i2c, int8_t controller)
{
    if (controller < 4) Seesaw.Init(i2c, controller);
    if (Seesaw.Present)
    {
        Type = ControllerTypes::Seesaw;
        Present = Seesaw.Present;
    }
    else Present = false;
}

void TinyCon::InputController::Update()
{
    int8_t axisIndex = 0;
    int8_t buttonIndex = 0;
    switch (Type)
    {
        case ControllerTypes::Pins:
            Pins.Update();
            for (float axis : Pins.Axis) Axis[axisIndex++] = axis;
            for (auto & button : Pins.Buttons) Buttons[buttonIndex++] = button.Get();
            break;
        case ControllerTypes::Seesaw:
            Seesaw.Update();
            for (float axis : Seesaw.Axis) Axis[axisIndex++] = axis;
            for (auto & button : Seesaw.Buttons) Buttons[buttonIndex++] = button.Get();
            break;
        default: break;
    }
}

bool TinyCon::InputController::GetUpdatedButton(int8_t index)
{
    switch (Type)
    {
        case ControllerTypes::Pins: return Pins.GetUpdatedButton(index);
        case ControllerTypes::Seesaw: return Seesaw.GetUpdatedButton(index);
        default: return false;
    }
}

TinyCon::ControllerTypes TinyCon::InputController::GetType() const
{
    switch (Type)
    {
        case ControllerTypes::Pins: return Pins.Present ? ControllerTypes::Pins : ControllerTypes::None;
        case ControllerTypes::Seesaw: return Seesaw.Present ? ControllerTypes::Seesaw : ControllerTypes::None;
        default: return ControllerTypes::None;
    }
}

int16_t TinyCon::InputController::GetAxisCount() const
{
    switch (Type)
    {
        case ControllerTypes::Pins: return Pins.GetAxisCount();
        case ControllerTypes::Seesaw: return Seesaw.Present ? sizeof(Seesaw.Axis) / sizeof(Seesaw.Axis[0]): 0;
        default: return 0;
    }
}

int16_t TinyCon::InputController::GetButtonCount() const
{
    switch (Type)
    {
        case ControllerTypes::Pins: return Pins.GetButtonCount();
        case ControllerTypes::Seesaw: return Seesaw.Present ? sizeof(Seesaw.Buttons) / sizeof(Seesaw.Buttons[0]): 0;
        default: return 0;
    }
}
