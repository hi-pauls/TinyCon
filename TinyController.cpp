#include "TinyController.h"

void TinyController::Init()
{
    Controller.Init();
    Power.Init();

#if !NO_SLAVE
    I2C.Init();
#endif

#if !NO_USB
    USBControl.Init();
    USBControl.SetActive(Power.PowerSource == PowerSources::USB);
    LOG_USB_LN("USB initialized");
#endif

#if !NO_BLE
    Bluetooth.Init();
    Bluetooth.SetActive(Power.PowerSource == PowerSources::Battery);
#endif
    Indicators.Init();
}

void TinyController::Update(int32_t deltaTime)
{
    bool i2cNeedsUpdate = Power.PowerSource == PowerSources::I2C;
    bool bluetoothWasConnected = Bluetooth.IsConnected();
    bool bluetoothNeedsUpdate = !i2cNeedsUpdate && Bluetooth.IsActive();
    bool usbNeedsUpdate = !i2cNeedsUpdate && !Bluetooth.IsConnected() && USBControl.IsActive();
    if (i2cNeedsUpdate || bluetoothNeedsUpdate || usbNeedsUpdate)
    {
        LOG_STATE_LN("State: Updating");
        Controller.Update(deltaTime);
        if (bluetoothNeedsUpdate) Bluetooth.Update(deltaTime);
        if (usbNeedsUpdate) USBControl.Update();
        UpdateSelectButton(deltaTime, Controller.GetButton(0, BluetoothStartButtonIndex));
        Suspended = false;
    }
    else
    {
        LOG_STATE_LN("State: Suspended");
        UpdateSelectButton(deltaTime, Controller.GetUpdatedButton(0, BluetoothStartButtonIndex));
        Suspended = true;
    }

    bool powerWasUsb = Power.PowerSource == PowerSources::USB;
    bool powerWasI2c = Power.PowerSource == PowerSources::I2C;
    Power.Update();
    bool powerIsUsb = Power.PowerSource == PowerSources::USB;
    if (Power.PowerSource == PowerSources::I2C)
    {
        LOG_STATE_LN("I2C connected, disable USB and Bluetooth (again)");
        USBControl.SetActive(false);
        Bluetooth.SetActive(false);
    }
    else if (powerWasI2c)
    {
        LOG_STATE("I2C disconnected, ");
        if (powerIsUsb)
        {
            LOG_STATE_LN("USB power, enabling USB, disabling Bluetooth");
            USBControl.SetActive(true);
            Bluetooth.SetActive(false);
        }
        else
        {
            LOG_STATE_LN("Battery power, enabling Bluetooth, disabling USB");
            USBControl.SetActive(false);
            Bluetooth.SetActive(true);
        }
    }
    else if (powerIsUsb)
    {
        if (!powerWasUsb)
        {
            LOG_STATE_LN("USB connected, disabling Bluetooth");
            USBControl.SetActive(true);
            Bluetooth.SetActive(false);
        }
        else if (bluetoothWasConnected && !Bluetooth.IsConnected())
        {
            LOG_STATE_LN("Forced Bluetooth on USB disconnect, enabling USB, stopping auto-advertising");
            USBControl.SetActive(true);
            Bluetooth.SetActive(false);
        }
    }
    else if (powerWasUsb)
    {
        LOG_STATE_LN("USB disconnected, enabling Bluetooth");
        USBControl.SetActive(false);
        Bluetooth.SetActive(true);
    }

    UpdateIndicators(deltaTime);
}

void TinyController::UpdateSelectButton(int32_t deltaTime, bool selectButton)
{
    if (selectButton)
        if (BluetoothStartPressedTimeout > 0) BluetoothStartPressedTimeout -= deltaTime;
        else
        {
            LOG_STATE_LN("Bluetooth Button Triggered");
            Bluetooth.SetActive(!Bluetooth.IsActive());
            if (!Bluetooth.IsActive() && Power.PowerSource == PowerSources::USB) USBControl.SetActive(true);
            BluetoothStartPressedTimeout = BluetoothStartButtonTime;
        }
    else BluetoothStartPressedTimeout = BluetoothStartButtonTime;
}

void TinyController::UpdateIndicators(int32_t deltaTime)
{
    if (Suspended) Indicators.Disable();
    else
    {
        if (Bluetooth.IsAdvertising()) Indicators.SetBlue(IndicatorController::LedEffects::Fade);
        else if (Bluetooth.IsConnected()) Indicators.SetBlue(IndicatorController::LedEffects::Pulse);
        else Indicators.SetBlue(IndicatorController::LedEffects::Off);

        if (Power.PowerSource == PowerSources::I2C) Indicators.SetRed(IndicatorController::LedEffects::Pulse);
        else if (Bluetooth.IsConnected() || Power.PowerSource != PowerSources::USB) Indicators.SetRed(IndicatorController::LedEffects::Off);
        else if (USBControl.IsConnected()) Indicators.SetRed(IndicatorController::LedEffects::On);
        else Indicators.SetRed(IndicatorController::LedEffects::Fade);

        char mode = 'D';
        if (Power.PowerSource == PowerSources::I2C) mode = 'I';
        else if (Bluetooth.IsConnected()) mode = 'B';
        else if (Bluetooth.IsAdvertising())
            if (USBControl.IsConnected()) mode = 'F';
            else mode = 'A';
        else if (USBControl.IsConnected()) mode = 'U';
        Indicators.Update(deltaTime, mode);
    }
}
