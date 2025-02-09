// Copyright 2025 Paul Schulze, All rights reserved.
#include "Config.h"

#include "Bluetooth.h"
#include "CommandProcessor.h"
#include "Indicators.h"
#include "HapticController.h"
#include "I2C.h"
#include "InputController.h"
#include "MpuController.h"
#include "Power.h"
#include "TinyController.h"
#include "USB.h"
#include "Utilities.h"

#ifdef NRF_CRYPTOCELL
#include <Adafruit_nRFCrypto.h>
#endif
#if !NO_USB
#include <Adafruit_TinyUSB.h>
#endif
#if !NO_BLE
#include <bluefruit.h>
#endif
#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <SoftWire.h>
#include <Wire.h>

#include <cstdint>

#if !NO_SLAVE
#include <nordic/nrfx/mdk/nrf52840.h>
TwoWire SlaveI2C(NRF_TWIM1, NRF_TWIS1, SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, 11, 12);
extern "C" { void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(void) { SlaveI2C.onService(); } }
#endif

TwoWire& MasterI2C0 = Wire;
SoftWire MasterI2C1{5, 0};
uint8_t MasterI2C1Buffer[10] = {};
TinyController Controller(SlaveI2C, 0x44, MasterI2C0, MasterI2C1);

#if USE_HAPTICTEST
uint8_t HapticCommand = 0;
    void UpdateHapticTest()
    {
    if (Controller.GetHapticQueueSize(HapticCommand >> 7) > 0)
    {
        uint8_t data[14];
        // Controller
        data[0] = HapticCommand >> 7;
        // Command
        data[1] = 1;
        // Size
        data[2] = 8;
        // Sub-command 0-7
        for (int8_t i = 0; i < 8; ++i, ++HapticCommand) data[3 + i] = HapticCommand & 0x7F;
        // Duration
        data[11] = 3000 >> 8;
        data[12] = 3000 & 0xFF;
        Controller.AddHapticCommand({data, 13});
    }
}
#else
inline void UpdateHapticTest() {}
#endif

void setup()
{
    Serial.begin(115200);

    MasterI2C0.begin();
    MasterI2C0.setTimeout(10);
    MasterI2C0.setClock(400000);

    MasterI2C1.begin();
    MasterI2C1.setRxBuffer(MasterI2C1Buffer, sizeof(MasterI2C1Buffer));
    MasterI2C1.setTxBuffer(MasterI2C1Buffer, sizeof(MasterI2C1Buffer));
    MasterI2C1.setTimeout_ms(10);
    MasterI2C1.setClock(400000);

#if !NO_SLAVE
    SlaveI2C.begin(0x44);
#endif

    Controller.Init();
    Watchdog.enable(1000);
}

constexpr auto UpdateFrequency = 1000 / 100;
uint32_t LastTime = 0;
void loop()
{
    auto time = millis();
    auto deltaTime = time - LastTime;
    LastTime = time;

    UpdateHapticTest();
    Controller.Update(deltaTime);

    Watchdog.reset();
    if (Controller.IsSuspended()) Watchdog.sleep(500);
    else
    {
        const auto updateTime = millis() - LastTime;
        if (updateTime < UpdateFrequency) Watchdog.sleep(UpdateFrequency - updateTime);
    }
}
