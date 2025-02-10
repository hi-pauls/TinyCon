#include "I2C.h"

#include "HapticController.h"

std::function<void(int)> I2CController::I2CReceiveCallback = [](int) {};
std::function<void()> I2CController::I2CRequestCallback = []() {};
void I2CController::I2CSlaveReceive(int count) { I2CReceiveCallback(count); }
void I2CController::I2CSlaveRequest() { I2CRequestCallback(); }

void I2CController::Init()
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
}

void I2CController::Send()
{
    int8_t commandAddress = 0;
    for (auto addr : CommandOrder)
        if (addr < RegisterAddress) commandAddress = addr;
        else break;

    uint8_t value = 0xA5;
    int8_t offset = RegisterAddress - commandAddress;
    switch (Command(commandAddress))
    {
        case Commands::ID:
            if (offset < 1) value = Controller.Id;
            break;
        case Commands::Version:
            if (offset < 1) value = GamepadController::Version;
            break;
        case Commands::Controller1: // 4
            if (offset == 0) value = ControllerTypeId(Controller.GetControllerType(0));
            else if (offset == 1) value = (Controller.GetAxisCount(0) << 5) | Controller.GetButtonCount(0);
            else if (offset == 2) value = MpuTypeId(Controller.GetMpuType(0));
            else if (offset == 3) value = HapticTypeId(Controller.GetHapticType(0));
            break;
        case Commands::Controller2: // 4
            if (offset == 0) value = ControllerTypeId(Controller.GetControllerType(1));
            else if (offset == 1) value = (Controller.GetAxisCount(1) << 5) | Controller.GetButtonCount(1);
            else if (offset == 2) value = MpuTypeId(Controller.GetMpuType(1));
            else if (offset == 3) value = HapticTypeId(Controller.GetHapticType(1));
            break;
        case Commands::LastCommand: // 3
            if (offset == 0) value = Processor.LastCommand;
            else if (offset == 1) value = Processor.LastParameter;
            else if (offset == 2) value = Processor.LastCommandStatus;
            break;
        case Commands::BatteryPercentage: // 2
            if (offset == 0) value = FloatToHalf(Power.Battery.Percentage) >> 8;
            else if (offset == 1) value = FloatToHalf(Power.Battery.Percentage) & 0xFF;
            break;
        case Commands::BatteryVoltage: // 2
            if (offset == 0) value = FloatToHalf(Power.Battery.Voltage) >> 8;
            else if (offset == 1) value = FloatToHalf(Power.Battery.Voltage) & 0xFF;
            break;
        case Commands::BatteryTemperature: // 2
            if (offset == 0) value = FloatToHalf(Power.Battery.Temperature) >> 8;
            else if (offset == 1) value = FloatToHalf(Power.Battery.Temperature) & 0xFF;
            break;
        case Commands::Controller1DeviceEnable:
            if (offset == 0)
            {
                value |= Controller.GetControllerEnabled(0) << 7;
                value |= Controller.GetAccelerationEnabled(0) << 5;
                value |= Controller.GetAngularVelocityEnabled(0) << 4;
                value |= Controller.GetOrientationEnabled(0) << 3;
                value |= Controller.GetTemperatureEnabled(0) << 2;
                value |= Controller.GetInputEnabled(0) << 1;
                value |= Controller.GetHapticEnabled(0);
            }
            break;
        case Commands::Controller2DeviceEnable:
            if (offset == 0)
            {
                value |= Controller.GetControllerEnabled(1) << 7;
                value |= Controller.GetAccelerationEnabled(1) << 5;
                value |= Controller.GetAngularVelocityEnabled(1) << 4;
                value |= Controller.GetOrientationEnabled(1) << 3;
                value |= Controller.GetTemperatureEnabled(1) << 2;
                value |= Controller.GetInputEnabled(1) << 1;
                value |= Controller.GetHapticEnabled(1);
            }
            break;
        case Commands::FeatureEnable:
            if (offset == 0)
            {
                value |= Processor.GetI2CEnabled() << 2;
                value |= Processor.GetBLEEnabled() << 1;
                value |= Processor.GetUSBEnabled();
            }
            break;
        case Commands::MpuSettings:
            if (offset == 0)
            {
                value = AccelerometerRangeId(Controller.GetAccelerometerRange()) << 4;
                value |= GyroscopeRangeId(Controller.GetGyroscopeRange());
            }
            break;
        case Commands::Haptic:
            if (BufferIndex < 3) SlaveI2C.write(0xFF);
            else
            {
                // Maximum 8 controllers with maximum 32 commands
                int8_t controller = Buffer[1];
                int8_t commandIndex = Buffer[2];
                if (offset == 0) value = HapticController::HapticCommandId(Controller.GetHapticCommand(controller, commandIndex));
                else if (offset == 1) value = Controller.GetHapticCommandCount(controller, commandIndex);
                else if (offset > 1 && offset < 10) value = Controller.GetHapticCommandData(controller, commandIndex, offset - 2);
                else if (offset == 10) value = Controller.GetHapticCommandDuration(controller, commandIndex) >> 8;
                else if (offset == 11) value = Controller.GetHapticCommandDuration(controller, commandIndex) & 0xFF;
            }
            break;
        case Commands::HapticQueueSize:
            if (BufferIndex < 2) SlaveI2C.write(0xFF);
            else if (offset == 0) value = Controller.GetHapticQueueSize(Buffer[1]);
            break;
        default:
            //if (offset < Controller.DataSize)
            //    value = Controller.Data[offset];
            break;
    }

    RegisterAddress++;
    SlaveI2C.write(value);
}

void I2CController::Receive()
{
    // Handle timeout first, if we havent received data for CommandTimeout us, reset
    auto time = micros() - LastInputTime;
    if (time > Timeout) BufferIndex = 0;

    if (!SlaveI2C.available()) return;

    // At least one byte must be received.
    Buffer[BufferIndex++] = SlaveI2C.read();
    RegisterAddress = Buffer[0];
    switch (Command(Buffer[0]))
    {
        case Commands::ID:
            if (BufferIndex > 1)
            {
                Processor.ProcessCommand({Buffer, BufferIndex});
                BufferIndex = 0;
            }
        case Commands::Version:
        case Commands::Controller1: // 4
        case Commands::Controller2: // 4
        case Commands::LastCommand: // 3
        case Commands::BatteryPercentage: // 2
        case Commands::BatteryVoltage: // 2
        case Commands::BatteryTemperature: // 2
            // Read-only commands, nothing to execute
            BufferIndex = 0;
            break;
        // Register set
        case Commands::Controller1DeviceEnable:
            if (BufferIndex > 1)
            {
                Processor.ProcessCommand({Buffer, BufferIndex});
                BufferIndex = 0;
            }

            break;
        case Commands::Controller2DeviceEnable:
            if (BufferIndex > 1)
            {
                Processor.ProcessCommand({Buffer, BufferIndex});
                BufferIndex = 0;
            }
            break;
        case Commands::FeatureEnable:
            if (BufferIndex > 1)
            {
                Processor.ProcessCommand({Buffer, BufferIndex});
                BufferIndex = 0;
            }
            break;
        case Commands::MpuSettings:
            if (BufferIndex > 1)
            {
                Processor.ProcessCommand({Buffer, BufferIndex});
                BufferIndex = 0;
            }
            break;
        case Commands::Haptic:
            if (BufferIndex > 14)
            {
                Processor.ProcessCommand({Buffer, 14});
                BufferIndex = 0;
            }
            break;
        case Commands::HapticQueueSize:
            if (BufferIndex > 1)
                BufferIndex = 0;
            break;
        case Commands::HapticRemove:
            if (BufferIndex > 2)
            {
                Processor.ProcessCommand({Buffer, BufferIndex});
                BufferIndex = 0;
            }
            break;
        case Commands::HapticReset:
            Processor.ProcessCommand({Buffer, BufferIndex});
            BufferIndex = 0;
            break;
        default:
            if (0x30 <= Buffer[0] && Buffer[0] < 0x30 + 82)
                BufferIndex = 0;
            break;
    }
}

