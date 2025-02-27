#pragma once

#include "Config.h"

#include "CommandProcessor.h"
#include "Utilities.h"

#include <Arduino.h>
#include <Wire.h>
#include <functional>

namespace TinyCon
{
    class I2CController
    {
    public:
        I2CController(TwoWire& slaveI2C, CommandProcessor& processor) : SlaveI2C(slaveI2C), Processor(processor) {}

        void Init();
        void Send();
        void Receive();

    private:
        static void I2CSlaveReceive(int count);
        static void I2CSlaveRequest();
        static std::function<void(int)> I2CReceiveCallback;
        static std::function<void()> I2CRequestCallback;

        TwoWire& SlaveI2C;
        CommandProcessor& Processor;

        uint8_t RegisterAddress = 0;
    };
}