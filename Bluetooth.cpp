#include "Config.h"
#include "Bluetooth.h"
#include "CommandProcessor.h"

#if !NO_BLE
std::function<void(uint16_t, BLECharacteristic*, uint8_t*, uint16_t)> BluetoothController::HapticWriteCallback =
        [](uint16_t, BLECharacteristic*, uint8_t*, uint16_t) {};
void BluetoothController::HapticWrite(uint16_t connection, BLECharacteristic *chr, uint8_t *data, uint16_t length)
{ HapticWriteCallback(connection, chr, data, length); }

void BluetoothController::Init()
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

    LOG_BLUETOOTH_LN("Bluetooth initialized");
}

void BluetoothController::Update(uint32_t deltaTime)
{
    LOG_BLUETOOTH("Bluetooth: ");
    if (!Processor.GetBLEEnabled() || !Active)
    {
        if (!Processor.GetBLEEnabled()) LOG_BLUETOOTH("Disabled");
        else LOG_BLUETOOTH("Inactive");

        if (Active || Connected)
        {
            Active = false;
            Connected = false;
            ForceAdvertise = false;
            AdvertisingStarted = false;
            Bluefruit.Advertising.stop();

            if (Bluefruit.connected())
            {
                LOG_BLUETOOTH(", Disconnect");
                Bluefruit.disconnect(ConnectionId);
            }

            if (Bluefruit.getTxPower() > -40) Bluefruit.setTxPower(-40);
            LOG_BLUETOOTH(", Stop");
        }
    }
    else if (ForceAdvertise || AdvertisingTimeout > 0)
    {
        if (AdvertisingTimeout <= 0 || !AdvertisingStarted)
        {
            // Previous advertising has timed out, reinitialize
            LOG_BLUETOOTH("Enable Advertising, ");
            Bluefruit.Advertising.start(0);
            AdvertisingStarted = true;
        }

        if (ForceAdvertise)
        {
            // One-time trigger flag
            LOG_BLUETOOTH("Force Advertise, ");
            ForceAdvertise = false;
            AdvertisingTimeout = AdvertisingTime;

            if (Bluefruit.connected())
            {
                LOG_BLUETOOTH("Disconnect, ");
                Bluefruit.disconnect(ConnectionId);
            }

            if (Bluefruit.getTxPower() < 4) Bluefruit.setTxPower(4);
            LOG_BLUETOOTH("Start");
        }
        else if (Bluefruit.connected())
        {
            // Connected during advertising, stop advertising
            LOG_BLUETOOTH("Connected, Stop Advertising");
            Bluefruit.Advertising.stop();
            AdvertisingStarted = false;
            AdvertisingTimeout = 0;
        }
        else
        {
            AdvertisingTimeout -= deltaTime;
            if (AdvertisingTimeout <= 0)
            {
                LOG_BLUETOOTH("Timeout");
                Bluefruit.Advertising.stop();
                AdvertisingStarted = false;
                Active = false;
            }
            else LOG_BLUETOOTH("Advertising");
        }
    }
    else if (Bluefruit.connected())
    {
        Connected = true;

        LOG_BLUETOOTH("Controller");
        ConnectionId = Bluefruit.connHandle();
        auto report = Controller.MakeHidReport();
        GamepadService.report(&report);

        LOG_BLUETOOTH(", MPU");
        uint8_t data[GamepadController::MaxControllers * 41];
        std::size_t size = Controller.MakeMpuBuffer({data, sizeof(data)});
        MpuCharacteristic.notify(data, size);
    }
    else
    {
        LOG_BLUETOOTH("Disconnected");
        ForceAdvertise = true;
    }

    LOG_BLUETOOTH_LN();
}

#endif
