#include "CommandProcessor.h"

void CommandProcessor::ProcessCommand(Span command)
{
    LastCommand = command.Data[0];
    LastParameter = command.Data[1];
    LastCommandStatus = 0;

    switch (Command(command.Data[0]))
    {
        case Commands::ID:
            Controller.Id = command.Data[1];
            break;
        case Commands::Controller1DeviceEnable:
            Controller.SetControllerEnabled(0, (command.Data[1] & 0x80) != 0);
            Controller.SetAccelerationEnabled(0, (command.Data[1] & 0x20) != 0);
            Controller.SetAngularVelocityEnabled(0, (command.Data[1] & 0x10) != 0);
            Controller.SetOrientationEnabled(0, (command.Data[1] & 0x08) != 0);
            Controller.SetTemperatureEnabled(0, (command.Data[1] & 0x04) != 0);
            Controller.SetInputEnabled(0, (command.Data[1] & 0x02) != 0);
            Controller.SetHapticEnabled(0, (command.Data[1] & 0x01) != 0);
            break;
        case Commands::Controller2DeviceEnable:
            Controller.SetControllerEnabled(1, (command.Data[1] & 0x80) != 0);
            Controller.SetAccelerationEnabled(1, (command.Data[1] & 0x20) != 0);
            Controller.SetAngularVelocityEnabled(1, (command.Data[1] & 0x10) != 0);
            Controller.SetOrientationEnabled(1, (command.Data[1] & 0x08) != 0);
            Controller.SetTemperatureEnabled(1, (command.Data[1] & 0x04) != 0);
            Controller.SetInputEnabled(1, (command.Data[1] & 0x02) != 0);
            Controller.SetHapticEnabled(1, (command.Data[1] & 0x01) != 0);
        break;
        case Commands::FeatureEnable:
            SetI2CEnabled((command.Data[1] & 0x4) != 0);
            SetBLEEnabled((command.Data[1] & 0x2) != 0);
            SetUSBEnabled((command.Data[1] & 0x1) != 0);
            break;
        case Commands::MpuSettings:
            Controller.SetAccelerometerRange(AccelerometerRange(command.Data[1] >> 4));
            Controller.SetGyroscopeRange(GyroscopeRange(command.Data[1] & 0xF));
            break;
        case Commands::Haptic:
            Controller.AddHapticCommand(command);
            break;
        case Commands::HapticRemove:
            Controller.RemoveHapticCommand(command.Data[1], command.Data[2]);
            break;
        case Commands::HapticReset:
            Controller.ClearHapticCommands();
            break;
        default:
            break;
    }

}
