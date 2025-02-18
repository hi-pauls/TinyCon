#include "Config.h"
#include "Bluetooth.h"
#include "CommandProcessor.h"

#if !NO_BLE
std::function<void(uint16_t, BLECharacteristic*, uint8_t*, uint16_t)> TinyCon::BluetoothController::HapticWriteCallback =
        [](uint16_t, BLECharacteristic*, uint8_t*, uint16_t) {};
void TinyCon::BluetoothController::HapticWrite(uint16_t connection, BLECharacteristic *chr, uint8_t *data, uint16_t length)
{ HapticWriteCallback(connection, chr, data, length); }

void TinyCon::BluetoothController::Init()
{
    Bluefruit.begin();
    Bluefruit.setTxPower(-40);
    Bluefruit.setName(TINYCON_PRODUCT);

    Discovery.setManufacturer(TINYCON_VENDOR);
    Discovery.setSoftwareRev(TINYCON_VERSION);
    Discovery.begin();

    GamepadService.begin();

    MpuService.begin();
    Bluefruit.Advertising.addService(MpuService);
    MpuCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
    MpuCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    MpuCharacteristic.setFixedLen(40);
    MpuCharacteristic.begin();

    HapticService.begin();
    Bluefruit.Advertising.addService(HapticService);
    HapticCharacteristic.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
    HapticCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    HapticWriteCallback = [this](uint16_t connection, BLECharacteristic* chr, uint8_t* data, uint16_t length)
        {
            if (length >= 1) Processor.ProcessCommand({data, length});
        };
    HapticCharacteristic.setWriteCallback(HapticWrite);
    HapticCharacteristic.begin();

    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);
    Bluefruit.Advertising.addService(GamepadService);
    Bluefruit.Advertising.addName();
    Bluefruit.Advertising.setInterval(32, 244);
    Bluefruit.Advertising.setFastTimeout(30);

    LogBluetooth::Info("Bluetooth initialized", Tiny::TIEndl);
}

void TinyCon::BluetoothController::Update(uint32_t deltaTime)
{
    LogBluetooth::Info("Bluetooth: ");
    if (!Processor.GetBLEEnabled() || !Active)
    {
        if (!Processor.GetBLEEnabled()) LogBluetooth::Info("Disabled");
        else LogBluetooth::Info("Inactive");

        if (Active || Connected)
        {
            Active = false;
            Connected = false;
            ForceAdvertise = false;
            AdvertisingStarted = false;
            Bluefruit.Advertising.stop();

            if (Bluefruit.connected())
            {
                LogBluetooth::Debug(", Disconnect");
                Bluefruit.disconnect(ConnectionId);
            }

            if (Bluefruit.getTxPower() > -40) Bluefruit.setTxPower(-40);
            LogBluetooth::Debug(", Stop");
        }
    }
    else if (ForceAdvertise || AdvertisingTimeout > 0)
    {
        if (AdvertisingTimeout <= 0 || !AdvertisingStarted)
        {
            // Previous advertising has timed out, reinitialize
            LogBluetooth::Debug("Enable Advertising, ");
            Bluefruit.Advertising.start(0);
            AdvertisingStarted = true;
        }

        if (ForceAdvertise)
        {
            // One-time trigger flag
            LogBluetooth::Debug("Force Advertise, ");
            ForceAdvertise = false;
            AdvertisingTimeout = AdvertisingTime;

            if (Bluefruit.connected())
            {
                LogBluetooth::Debug("Disconnect, ");
                Bluefruit.disconnect(ConnectionId);
            }

            if (Bluefruit.getTxPower() < 4) Bluefruit.setTxPower(4);
            LogBluetooth::Debug("Start");
        }
        else if (Bluefruit.connected())
        {
            // Connected during advertising, stop advertising
            LogBluetooth::Debug("Connected, Stop Advertising");
            Bluefruit.Advertising.stop();
            AdvertisingStarted = false;
            AdvertisingTimeout = 0;
        }
        else
        {
            AdvertisingTimeout -= deltaTime;
            if (AdvertisingTimeout <= 0)
            {
                LogBluetooth::Debug("Timeout");
                Bluefruit.Advertising.stop();
                AdvertisingStarted = false;
                Active = false;
            }
            else LogBluetooth::Info("Advertising");
        }
    }
    else if (Bluefruit.connected())
    {
        Connected = true;

        LogBluetooth::Debug("Controller");
        ConnectionId = Bluefruit.connHandle();
        auto report = Controller.MakeHidReport();
        GamepadService.report(&report);

        LogBluetooth::Debug(", MPU");
        uint8_t data[GamepadController::MaxControllers * 41];
        std::size_t size = Controller.MakeMpuBuffer({data, sizeof(data)});
        MpuCharacteristic.notify(data, size);
    }
    else
    {
        LogBluetooth::Debug("Disconnected");
        ForceAdvertise = true;
    }

    LogBluetooth::Info(Tiny::TIEndl);
}

#endif
