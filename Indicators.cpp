#include "Indicators.h"

void IndicatorController::Init()
{
    pinMode(BlueLedPin, OUTPUT);
    pinMode(RedLedPin, OUTPUT);
    LOG_DISPLAY_LN("Indicators initialized");
}

void IndicatorController::Update(uint32_t deltaTime, char mode)
{
    UpdateBlue(deltaTime);
    UpdateRed(deltaTime);

#if USE_NEOPIXEL
    UpdateRgb(deltaTime);
#endif

#if USE_OLED
    UpdateDisplay(mode);
#endif
}

#if USE_OLED
void IndicatorController::UpdateDisplay(char mode)
{
    if (!DisplayPresent)
    {
        LOG_DISPLAY_LN("Init");
        DisplayPresent = SSD1306.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        if (DisplayPresent) LOG_DISPLAY(" Success");
        else LOG_DISPLAY(" Failed");
    }

    if (DisplayPresent)
    {
        if (Suspended)
        {
            SSD1306.dim(0);
            Suspended = false;
        }

        SSD1306.clearDisplay();

        auto axisCount = 0;
        int8_t buttonCount = 0;
        for (auto i = 0; i < GamepadController::MaxControllers; ++i)
        {
            axisCount = Max(axisCount, Controller.GetAxisCount(i));
            buttonCount = Max(buttonCount, Controller.GetButtonCount(i));
        }

        auto axisHeight = (ControllerHeight + axisCount - 1) / axisCount;
        auto buttonWidth = ControllerHeight + 1;
        auto buttonHeight = ControllerHeight;
        auto requiredButtonsPerLine = buttonCount;
        while (buttonWidth * requiredButtonsPerLine > ButtonEnd - ButtonStart)
        {
            if (buttonWidth >= buttonHeight) buttonWidth--;
            else { buttonHeight--; buttonWidth++; }
            auto lines = ControllerHeight / buttonHeight;
            requiredButtonsPerLine = (buttonCount + lines - 1) / lines;
        }

        for (int8_t c = 0, controllerY = 0; c < GamepadController::MaxControllers; ++c, controllerY += ControllerHeight) {
            for (int8_t i = 0, axisY = controllerY; i < axisCount; ++i, ++axisY)
                if (i < Controller.GetAxisCount(c)) {
                    auto axis = Controller.GetAxis(c, i);
                    auto w = Max(abs(AxisRoot * axis), 1);
                    auto x = axis < 0 ? AxisRoot - w : AxisRoot;
                    for (auto y = 0; y < axisHeight - 1; ++y, ++axisY)
                        SSD1306.drawFastHLine(x, axisY, w, 1);
                }

            auto buttonY = controllerY + (ControllerHeight - buttonHeight) / 2;
            for (int8_t i = 0, buttonX = ButtonStart; i < buttonCount; ++i, buttonX += buttonWidth) {
                if (buttonX + buttonWidth > ButtonEnd) {
                    buttonX = ButtonStart;
                    buttonY += buttonHeight;
                }

                if (i < Controller.GetButtonCount(c) && Controller.GetButton(c, i))
                    for (auto y = 0; y < buttonHeight - 1; ++y)
                        SSD1306.drawFastHLine(buttonX, buttonY + y, buttonWidth - 1, 1);
            }

            if (Controller.GetMpuPresent(c))
            {
                // MPU has 3x3 axis per controller, + and - for each axis, using vertical bars spanning the given width
                auto mpuCenter = ControllerHeight / 2;
                auto barWidth = MpuWidth / (3 * 3);
                auto acceleration = Controller.GetAcceleration(c);
                auto angularVelocity = Controller.GetAngularVelocity(c);
                auto orientation = Controller.GetOrientation(c);
                int8_t bars[] =
                    {
                        static_cast<int8_t>(controllerY + mpuCenter + Min(AccelerationScale, Max(-AccelerationScale, acceleration.X/ AccelerationScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(AccelerationScale, Max(-AccelerationScale, acceleration.Y/ AccelerationScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(AccelerationScale, Max(-AccelerationScale, acceleration.Z/ AccelerationScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(AngularVelocityScale, Max(-AngularVelocityScale, angularVelocity.X/ AngularVelocityScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(AngularVelocityScale, Max(-AngularVelocityScale, angularVelocity.Y/ AngularVelocityScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(AngularVelocityScale, Max(-AngularVelocityScale, angularVelocity.Z/ AngularVelocityScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(OrientationScale, Max(-OrientationScale, orientation.X/ OrientationScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(OrientationScale, Max(-OrientationScale, orientation.Y/ OrientationScale)) * mpuCenter),
                        static_cast<int8_t>(controllerY + mpuCenter + Min(OrientationScale, Max(-OrientationScale, orientation.Z/ OrientationScale)) * mpuCenter),
                    };
                mpuCenter += controllerY;
                for (uint32_t i = 0, barX = MpuStart; i < sizeof(bars) / sizeof(bars[0]); ++i, barX += barWidth)
                {
                    auto from = Min(bars[i], mpuCenter);
                    auto to = Min(Max(bars[i], mpuCenter), controllerY + ControllerHeight - 1);
                    if (from >= to) to = from + 1;
                    for (auto y = from; y < to; ++y) SSD1306.drawFastHLine(barX, y, barWidth - 1, 1);
                }
            }
        }

        // 120 - 127 Connection and Haptic Buffer Status
        SSD1306.setCursor(120, 0);
        SSD1306.setTextColor(1);
        SSD1306.print(mode);
        SSD1306.setCursor(120, 8);
        if (Power.PowerSource == PowerSources::USB)
        {
            if (Power.Battery.Voltage < 4.1f) SSD1306.print('C');
            else SSD1306.print('F');
        }
        else if (Power.PowerSource == PowerSources::Battery) SSD1306.print('B');
        SSD1306.setCursor(120, 16);
        SSD1306.print(Controller.GetHapticQueueSize(0));
        SSD1306.setCursor(120, 24);
        SSD1306.print(Controller.GetHapticQueueSize(1));
        SSD1306.display();
    }
}

void IndicatorController::Disable()
{
    if (!Suspended)
    {
#if USE_NEOPIXEL
        SetRgb(LedEffects::Off, LedEffects::Off, LedEffects::Off);
        UpdateRgb(0);
#endif

        analogWrite(BlueLedPin, 0);
        analogWrite(RedLedPin, 0);

#if USE_OLED
        SSD1306.clearDisplay();
        SSD1306.display();
        SSD1306.dim(1);
#endif
        Suspended = true;
    }
}
#endif

void IndicatorController::UpdateLed(uint32_t deltaTime, uint8_t& value, int32_t& time, LedEffects& current, LedEffects& next)
{
    time -= deltaTime;
    switch (current)
    {
        case LedEffects::Off:
            value = 0;
            if (next != LedEffects::Off)
            {
                current = next;
                time = 2000;
            }
            break;
        case LedEffects::On:
            value = 127;
            if (next != LedEffects::On)
            {
                current = next;
                time = 2000;
            }
            break;
        case LedEffects::Pulse:
            if (next != LedEffects::Pulse)
            {
                current = next;
                time = 2000;
            }
            else if (time < 0)
            {
                if (value == 0)
                {
                    time = 50;
                    value = 127;
                }
                else
                {
                    time = 5000;
                    value = 0;
                }
            }
            break;
        case LedEffects::Fade:
            if (next != LedEffects::Fade)
            {
                current = next;
                time = 2000;
            }
            else if (time < 0) time = 2000;
            else if (time < 1000) value = time >> 3;
            else value = 127 - ((time - 1000) >> 3);
            break;
    }
}

void IndicatorController::UpdateBlue(uint32_t deltaTime)
{
    UpdateLed(deltaTime, BlueEffectValue, BlueEffectTime, BlueEffect, NextBlueEffect);
    analogWrite(BlueLedPin, BlueEffectValue);
}

void IndicatorController::UpdateRed(uint32_t deltaTime)
{
    UpdateLed(deltaTime, RedEffectValue, RedEffectTime, RedEffect, NextRedEffect);
    analogWrite(RedLedPin, RedEffectValue);
}

#if USE_NEOPIXEL
void DisplayController::UpdateRgb(uint32_t deltaTime)
{
    UpdateLed(deltaTime, RgbRedValue, RgbRedEffectTime, RgbRedEffect, NextRgbRedEffect);
    UpdateLed(deltaTime, RgbGreenValue, RgbGreenEffectTime, RgbGreenEffect, NextRgbGreenEffect);
    UpdateLed(deltaTime, RgbBlueValue, RgbBlueEffectTime, RgbBlueEffect, NextRgbBlueEffect);
    SetRgb(RgbRedValue, RgbGreenValue, RgbBlueValue);
}
#endif
