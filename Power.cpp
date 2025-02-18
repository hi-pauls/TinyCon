#include "Power.h"

using LogPower = Tiny::TILogTarget<TinyCon::PowerLogLevel>;
{
#if defined(ADAFRUIT_FEATHER_ESP32S2)
    pinMode(PIN_I2C_POWER, INPUT);
    delay(1);
    bool polarity = digitalRead(PIN_I2C_POWER);
    pinMode(PIN_I2C_POWER, OUTPUT);
    digitalWrite(PIN_I2C_POWER, !polarity);
#endif

#ifndef ESP32
    analogReference(AR_INTERNAL_3_0);
#endif
    pinMode(USBAdcPin, INPUT);
    pinMode(BatteryAdcPin, INPUT);

#if USE_LC709203
    if ((LC709203FPresent = LC709203F.begin(i2c)))
    {
        LC709203F.setThermistorB(3950);
        LC709203F.setPackSize(LC709203F_APA_100MAH);
        LC709203F.setAlarmVoltage(3.8);
    }
#endif

    Update();
}

void PowerController::Update()
{
    PowerSource = PowerSources::I2C;

#if USE_LC709203
    if (LC709203FPresent)
    {
        Battery.Voltage = LC709203F.cellVoltage();
        Battery.Percentage = LC709203F.cellPercent();
    }
    else
#endif
    if constexpr (BatteryAdcPin >= 0)
    {
        Battery.Voltage = analogRead(BatteryAdcPin) * (3.0f / 1024) * 2;
        Battery.Percentage = 2.0f / Battery.Voltage - pow(4.3f - Battery.Voltage, 2) + 0.55f;
        Battery.Percentage = min(1.0f, max(0.0f, Battery.Percentage));
        if (Battery.Voltage > BatteryPresentVoltage) PowerSource = PowerSources::Battery;
    }

    if constexpr (USBAdcPin >= 0)
    {
        USBPowerVoltage = analogRead(USBAdcPin) * (3.0f / 1024) * 2;
        if (USBPowerVoltage > USBPowerPresentVoltage) PowerSource = PowerSources::USB;
    }
    LogPower::Info("Power: ", PowerSource == PowerSources::USB ? "USB" : PowerSource == PowerSources::Battery ? "Battery" : "I2C",
                    ", USB: ", USBPowerVoltage, 2, "V, Battery: ", Battery.Voltage, 2, "V, ", Battery.Percentage * 100, 0, "%", Tiny::TIEndl);
}
