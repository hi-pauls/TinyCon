#pragma once

#include "Config.h"

#include "CommandProcessor.h"
#include "GamepadController.h"
#include "Power.h"
#include "Utilities.h"

#include <Arduino.h>
#include <Wire.h>
#include <functional>

class I2CController
{
public:
    I2CController(TwoWire& slaveI2C, const GamepadController& controller, const PowerController& power, CommandProcessor& processor)
        : SlaveI2C(slaveI2C), Controller(controller), Power(power), Processor(processor) {}

    void Init();
    void Send();
    void Receive();

private:
    static void I2CSlaveReceive(int count);
    static void I2CSlaveRequest();
    static std::function<void(int)> I2CReceiveCallback;
    static std::function<void()> I2CRequestCallback;

    TwoWire& SlaveI2C;
    const GamepadController& Controller;
    const PowerController& Power;
    CommandProcessor& Processor;

    uint8_t RegisterAddress = 0;
    uint32_t Timeout = 200;
    uint32_t LastInputTime = 0;
    uint8_t Buffer[14] = {};
    uint8_t BufferIndex = 0;
};
