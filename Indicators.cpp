#include "Indicators.h"

using LogIndicators = Tiny::TILogTarget<TinyCon::IndicatorLogLevel>;

void TinyCon::IndicatorController::Init()
{
    pinMode(BlueLedPin, OUTPUT);
    pinMode(RedLedPin, OUTPUT);
    LogIndicators::Debug("Indicators initialized", Tiny::TIEndl);
}

void TinyCon::IndicatorController::Update(uint32_t deltaTime, char mode)
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
void TinyCon::IndicatorController::UpdateDisplay(char mode)
{
    if (!DisplayPresent)
    {
        LogIndicators::Debug("Init", Tiny::TIEndl);
        DisplayPresent = SSD1306.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        if (DisplayPresent) LogIndicators::Debug(" Success");
        else LogIndicators::Debug(" Failed");
    }

    if (DisplayPresent)
    {
        if (Suspended)
        {
            SSD1306.dim(0);
            Suspended = false;
        }

        SSD1306.clearDisplay();

        auto axisCount = Controller.GetAxisCount();
        auto axisHeight = axisCount > 0 ? DisplayHeight / axisCount : 0;
        auto buttonWidth = DisplayHeight;
        auto buttonHeight = DisplayHeight;
        auto buttonCount = Controller.GetButtonCount();
        auto requiredButtonsPerLine = buttonCount;
        while (buttonWidth * requiredButtonsPerLine > ButtonEnd - ButtonStart)
        {
            if (buttonWidth >= buttonHeight) buttonWidth--;
            else { buttonHeight--; buttonWidth++; }
            auto lines = 32 / buttonHeight;
            requiredButtonsPerLine = (buttonCount + lines - 1) / lines;
        }

        for (int8_t i = 0, axisY = 0; i < axisCount; ++i, ++axisY)
        {
            auto axis = Controller.GetAxis(i);
            auto w = Tiny::Math::Max(abs(AxisRoot * axis), 1);
            auto x = axis < 0 ? AxisRoot - w : AxisRoot;
            for (auto y = 0; y < axisHeight - 1; ++y, ++axisY)
                SSD1306.drawFastHLine(x, axisY, w, 1);
        }

        for (int8_t i = 0, buttonX = ButtonStart, buttonY = 0; i < buttonCount; ++i, buttonX += buttonWidth)
        {
            if (buttonX + buttonWidth > ButtonEnd)
            {
                buttonX = ButtonStart;
                buttonY += buttonHeight;
            }

            if (Controller.GetButton(i))
                for (auto y = 0; y < buttonHeight - 1; ++y)
                    SSD1306.drawFastHLine(buttonX, buttonY + y, buttonWidth - 1, 1);
        }

        auto mpuCount = Controller.GetMpuCount();
        auto mpuHeight = mpuCount > 0 ? DisplayHeight / mpuCount : 0;
        for (int8_t mpuY = 0, mpuIndex = 0; mpuIndex < GamepadController::MaxMpuControllers; mpuY += mpuHeight, ++mpuIndex)
        {
            for (;!Controller.GetMpuPresent(mpuIndex) && mpuIndex < GamepadController::MaxMpuControllers; mpuIndex++);

            if (Controller.GetMpuPresent(mpuIndex))
            {
                // MPU has 3x3 axis per controller, + and - for each axis, using vertical bars spanning the given width
                auto mpuCenter = mpuHeight / 2;
                auto barWidth = MpuWidth / (3 * 3);
                auto acceleration = Controller.GetAcceleration(mpuIndex);
                auto angularVelocity = Controller.GetAngularVelocity(mpuIndex);
                auto orientation = Controller.GetOrientation(mpuIndex);
                int8_t bars[] =
                    {
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(AccelerationScale, Tiny::Math::Max(-AccelerationScale, acceleration.X/ AccelerationScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(AccelerationScale, Tiny::Math::Max(-AccelerationScale, acceleration.Y/ AccelerationScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(AccelerationScale, Tiny::Math::Max(-AccelerationScale, acceleration.Z/ AccelerationScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(AngularVelocityScale, Tiny::Math::Max(-AngularVelocityScale, angularVelocity.X/ AngularVelocityScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(AngularVelocityScale, Tiny::Math::Max(-AngularVelocityScale, angularVelocity.Y/ AngularVelocityScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(AngularVelocityScale, Tiny::Math::Max(-AngularVelocityScale, angularVelocity.Z/ AngularVelocityScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(OrientationScale, Tiny::Math::Max(-OrientationScale, orientation.X/ OrientationScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(OrientationScale, Tiny::Math::Max(-OrientationScale, orientation.Y/ OrientationScale)) * mpuCenter),
                        static_cast<int8_t>(mpuY + mpuCenter + Tiny::Math::Min(OrientationScale, Tiny::Math::Max(-OrientationScale, orientation.Z/ OrientationScale)) * mpuCenter),
                    };
                mpuCenter += mpuY;
                for (uint32_t i = 0, barX = MpuStart; i < sizeof(bars) / sizeof(bars[0]); ++i, barX += barWidth)
                {
                    auto from = Tiny::Math::Min(bars[i], mpuCenter);
                    auto to = Tiny::Math::Min(Tiny::Math::Max(bars[i], mpuCenter), mpuY + mpuHeight - 1);
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

void TinyCon::IndicatorController::Disable()
{
    if (!Suspended)
    {
#if USE_NEOPIXEL
        SetRgbRed(LedEffects::Off);
        SetRgbGreen(LedEffects::Off);
        SetRgbBlue(LedEffects::Off);
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

void TinyCon::IndicatorController::UpdateLed(uint32_t deltaTime, uint8_t& value, int32_t& time, LedEffects& current, LedEffects& next)
{
    time -= deltaTime;
    switch (current)
    {
        case LedEffects::Off:
            if (next != LedEffects::Off)
            {
                current = next;
                time = 2000;
            }
            else value = 0;
            break;
        case LedEffects::On:
            if (next != LedEffects::On)
            {
                current = next;
                time = 2000;
            }
            else value = 127;
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
        case LedEffects::Fixed:
            if (next != LedEffects::Fixed)
            {
                current = next;
                time = 2000;
            }

            break;
    }
}

void TinyCon::IndicatorController::UpdateBlue(uint32_t deltaTime)
{
    UpdateLed(deltaTime, BlueEffectValue, BlueEffectTime, BlueEffect, NextBlueEffect);
    analogWrite(BlueLedPin, BlueEffectValue);
}

void TinyCon::IndicatorController::UpdateRed(uint32_t deltaTime)
{
    UpdateLed(deltaTime, RedEffectValue, RedEffectTime, RedEffect, NextRedEffect);
    analogWrite(RedLedPin, RedEffectValue);
}

#if USE_NEOPIXEL
void TinyCon::IndicatorController::UpdateRgb(uint32_t deltaTime)
{
    UpdateLed(deltaTime, RgbRedValue, RgbRedEffectTime, RgbRedEffect, NextRgbRedEffect);
    UpdateLed(deltaTime, RgbGreenValue, RgbGreenEffectTime, RgbGreenEffect, NextRgbGreenEffect);
    UpdateLed(deltaTime, RgbBlueValue, RgbBlueEffectTime, RgbBlueEffect, NextRgbBlueEffect);
    SetRgb(RgbRedValue, RgbGreenValue, RgbBlueValue);
}
void TinyCon::IndicatorController::SetRgbColor(TinyCon::IndicatorController::LedEffects nextEffect, uint8_t nextValue,
                                               TinyCon::IndicatorController::LedEffects &currentEffect, uint8_t &currentValue)
{
    currentEffect = nextEffect;
    if (nextEffect == LedEffects::Fixed) currentValue = nextValue;
}
#endif
