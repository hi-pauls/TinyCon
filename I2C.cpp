#include "I2C.h"

#include "HapticController.h"

using LogI2C = Tiny::TILogTarget<TinyCon::I2CLogLevel>;

std::function<void(int)> TinyCon::I2CController::I2CReceiveCallback = [](int) {};
std::function<void()> TinyCon::I2CController::I2CRequestCallback = []() {};
void TinyCon::I2CController::I2CSlaveReceive(int count) { I2CReceiveCallback(count); }
void TinyCon::I2CController::I2CSlaveRequest() { I2CRequestCallback(); }

void TinyCon::I2CController::Init()
{
    I2CReceiveCallback = [this](int count)
        {
            if (Processor.GetI2CEnabled()) for (auto i = 0; i < count; ++i) Receive();
        };
    I2CRequestCallback = [this]()
        {
            if (Processor.GetI2CEnabled()) Send();
        };
    SlaveI2C.onRequest(I2CSlaveRequest);
    SlaveI2C.onReceive(I2CSlaveReceive);

    // This ensures, that each register byte we don't write will read back the address of the register, providing a
    // good way to determine which registers are being read properly and where a read is potentially accessing the
    // wrong register.
    for (auto i = 0; i < MaxI2CRegisters; ++i) Registers[i] = i & 0xFF;

    // Set the read-only registers with static data and initialize the others with the defaults
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::ID, Controller.Id);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Version, 0, GamepadController::Version >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Version, 1, GamepadController::Version & 0xFF);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Magic, 0, GamepadController::Magic >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Magic, 1, GamepadController::Magic & 0xFF);

    // Initialize the configurable registers with the default settings
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::MPUDataEnable,
                Controller.GetAccelerationEnabled() << 3 | Controller.GetAngularVelocityEnabled() << 2 |
                Controller.GetOrientationEnabled() << 1 | Controller.GetTemperatureEnabled());
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::FeatureEnable,
                Processor.GetI2CEnabled() << 2 | Processor.GetBLEEnabled() << 1 | Processor.GetUSBEnabled());

    for (int8_t i = 0; i < GamepadController::MaxMpuControllers; ++i)
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::MpuConfig1, i,
                    AccelerometerRangeId(Controller.GetAccelerometerRange(i)) << 4 | GyroscopeRangeId(Controller.GetGyroscopeRange(i)));

    // Initialize the dynamic data
    Update();
}

/**
 * Write the data to be read from the I2C bus and queue it to the Wire TX buffer to be read by the master. This will
 * always re-read the last address written to the address register and will consume any data that would otherwise be
 * used for write commands, so additional reads will not be used as write parameters. Because the Wire library does not
 * allow us to properly detect stop conditions or read back the remaining bytes in the TX buffer, it is impossible to
 * determine the actual number of sent bytes. For this reason, we opt to not auto-increment the register base address
 * with each queued byte, but instead expect the master to update the read base address and read consecutive data until
 * done or up to the maximum SERIAL_BUFFER_SIZE, then set a new address to continue. Yes, overkill, no choice.
 *
 * A read also always invalidates the previous write, so the following commands are not treated as parameter data.
 */
void TinyCon::I2CController::Send()
{
    LogI2C::Debug("I2C Read: ", RegisterAddress, Tiny::TIEndl);
    SlaveI2C.write(Registers.data() + RegisterAddress, TinyCon::MaxI2CWriteBufferFill);
    BufferIndex = 0;
}

void TinyCon::I2CController::Receive()
{
    // Handle timeout first, if we haven't received data for CommandTimeout us, reset
    auto time = micros();
    if (time - LastInputTime > Timeout) BufferIndex = 0;
    LastInputTime = time;

    if (!SlaveI2C.available()) return;

    // At least one byte must be received.
    Buffer[BufferIndex++] = SlaveI2C.read();
    RegisterAddress = Buffer[0];
    switch (Tiny::Drivers::Input::TITinyConCommand(Buffer[0]))
    {
        // Handle writable commands, these can only reset the buffer index if complete
        // or timed out, only then can we also update the register address
        case Tiny::Drivers::Input::TITinyConCommands::ID:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig1:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig2:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig3:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig4:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig5:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig6:
        case Tiny::Drivers::Input::TITinyConCommands::MPUDataEnable:
        case Tiny::Drivers::Input::TITinyConCommands::FeatureEnable:
            // These are 1-byte writes, if they succeed, just copy them to the register for reading.
            if (Processor.ProcessCommand({Buffer.data(), BufferIndex}))
            {
                Registers[RegisterAddress] = Buffer[1];
                BufferIndex = 0;
            }

            break;
        case Tiny::Drivers::Input::TITinyConCommands::HapticRemove:
        case Tiny::Drivers::Input::TITinyConCommands::HapticReset:
            // Since the result of these can't be read back immediately but needs to be read through
            // the haptic command, we don't need to store any change in the register map.
            if (Processor.ProcessCommand({Buffer.data(), BufferIndex})) BufferIndex = 0;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::Haptic:
            if (Processor.ProcessCommand({Buffer.data(), BufferIndex})) BufferIndex = 0;
            else if (BufferIndex == 1) memset(Registers.data() + RegisterAddress, 0xFF, 12);
            else if (BufferIndex > 2 && Buffer[1] < GamepadController::MaxHapticControllers && Buffer[2] < 8)
            {
                int8_t controller = Buffer[1];
                int8_t queueIndex = Buffer[2];
                Registers[RegisterAddress] = HapticController::HapticCommandId(Controller.GetHapticCommand(controller, queueIndex));
                Registers[RegisterAddress + 1] = Controller.GetHapticCommandCount(controller, queueIndex);
                for (int8_t i = 0; i < 8; ++i) Registers[RegisterAddress + 2 + i] = Controller.GetHapticCommandData(controller, queueIndex, i);
                Registers[RegisterAddress + 10] = Controller.GetHapticCommandDuration(controller, queueIndex) >> 8;
                Registers[RegisterAddress + 11] = Controller.GetHapticCommandDuration(controller, queueIndex) & 0xFF;
            }

            break;
        case Tiny::Drivers::Input::TITinyConCommands::HapticQueueSize:
            // This and the haptic command are the only registers that require additional data
            // to be read. The haptic command does require 14 byte anyway, so this is fine,
            // but the queue size command needs this special case.
            if (BufferIndex == 1) Registers[RegisterAddress] = 0xFF;
            else if (BufferIndex > 1)
            {
                Registers[RegisterAddress] = Controller.GetHapticQueueSize(Buffer[1]);
                BufferIndex = 0;
            }

            break;
        default:
            // Any non-writable command is assumed to reset the buffer immediately
            BufferIndex = 0;
            break;
    }

    // This needs to update every time we receive a byte, users could be reading back the status at any time
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 0, Processor.LastCommand);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 1, Processor.LastParameter[0]);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 2, Processor.LastParameter[1]);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 3, Tiny::Drivers::Input::TITinyConCommandStatusId(Processor.LastCommandStatus));
}

void TinyCon::I2CController::Update()
{
    uint16_t vinVoltage = Tiny::Math::HalfFromFloat(Power.USBPowerVoltage);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::VinVoltage, 0, vinVoltage >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::VinVoltage, 1, vinVoltage & 0xFF);
    uint16_t batteryPercentage = Tiny::Math::HalfFromFloat(Power.Battery.Percentage);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::BatteryPercentage, 0, batteryPercentage >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::BatteryPercentage, 1, batteryPercentage & 0xFF);
    uint16_t batteryVoltage = Tiny::Math::HalfFromFloat(Power.Battery.Voltage);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::BatteryVoltage, 0, batteryVoltage >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::BatteryVoltage, 1, batteryVoltage & 0xFF);
    uint16_t batteryTemperature = Tiny::Math::HalfFromFloat(Power.Battery.Temperature);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::BatteryTemperature, 0, batteryTemperature >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::BatteryTemperature, 1, batteryTemperature & 0xFF);

    // Because devices may be detected any time, we need to update the types registers every time with the data
    for (int8_t i = 0; i < GamepadController::MaxHapticControllers; ++i)
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::HapticTypes, i, HapticTypeId(Controller.GetHapticType(i)));
    for (int8_t i = 0; i < GamepadController::MaxInputControllers; ++i)
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::ControllerTypes, i,
                    ControllerTypeId(Controller.GetControllerType(i)));

    SetRegister(Tiny::Drivers::Input::TITinyConCommands::AxisCount, Controller.GetAxisCount());
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::ButtonCount, Controller.GetButtonCount());
    auto dataOffset = Tiny::Drivers::Input::TITinyConCommandId(Tiny::Drivers::Input::TITinyConCommands::Data);
    for (int8_t i = 0; i < GamepadController::MaxMpuControllers; ++i)
    {
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::MpuTypes, i, MpuTypeId(Controller.GetMpuType(i)));
        if (Controller.GetMpuPresent(i)) dataOffset += Controller.MakeMpuBuffer({Registers.data() + dataOffset, 20});
    }

    uint8_t value = 0;
    for (int8_t i = 0; i < Controller.GetButtonCount(); ++i)
    {
        value |= Controller.GetButton(i) << (i & 7);
        if ((i & 7) == 7)
        {
            SetRegister(Tiny::Drivers::Input::TITinyConCommands::Data, dataOffset++, value);
            value = 0;
        }
    }

    for (auto i = 0; i < Controller.GetAxisCount(); ++i)
    {
        auto axis = Tiny::Math::HalfFromFloat(Controller.GetAxis(i));
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::Data, dataOffset++, axis >> 8);
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::Data, dataOffset++, axis & 0xFF);
    }
}