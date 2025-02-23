#include "CommandProcessor.h"

using LogCommand = Tiny::TILogTarget<TinyCon::CommandLogLevel>;

bool TinyCon::CommandProcessor::ProcessCommand(Tiny::Collections::TIFixedSpan<uint8_t> command)
{
    LastCommand = command[0];
    LastParameter[0] = command[1];
    LastParameter[1] = command[2];
    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidCommand;

    switch (Tiny::Drivers::Input::TITinyConCommands(command[0]))
    {
        case Tiny::Drivers::Input::TITinyConCommands::ID:
            if (command.size() >= 2)
            {
                Controller.Id = command[1];
                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                return true;
            }

            break;
        case Tiny::Drivers::Input::TITinyConCommands::Haptic:
            if (command.size() >= 13)
            {
                if (command[2] > 8)
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticDataSize;
                    LogCommand::Error("Invalid Haptic Data Size: ", command[2], Tiny::TIEndl);
                    return false;
                }

                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                for (auto bit = GamepadController::MaxHapticControllers; bit < 8; ++bit)
                    if ((command[1] & (1 << bit)) != 0)
                    {
                        // We'll still execute the command, but we'll return an error
                        LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::WarningUnknownHapticController;
                        LogCommand::Warning("Unknown Haptic Controller: ", bit, Tiny::TIEndl);
                    }

                Controller.AddHapticCommand(command);
                return true;
            }

            break;
        case Tiny::Drivers::Input::TITinyConCommands::HapticRemove:
            if (command.size() >= 3)
            {
                if (command[1] >= GamepadController::MaxHapticControllers)
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticController;
                    LogCommand::Error("Invalid Haptic Controller: ", command[1], Tiny::TIEndl);
                    return false;
                }

                if (command[2] >= 8)
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticDataIndex;
                    LogCommand::Error("Invalid Haptic Data Index: ", command[2], Tiny::TIEndl);
                    return false;
                }

                Controller.RemoveHapticCommand(command[1], command[2]);
                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                return true;
            }

            break;
        case Tiny::Drivers::Input::TITinyConCommands::HapticReset:
            Controller.ClearHapticCommands();
            LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
            return true;
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig1:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig2:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig3:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig4:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig5:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig6:
            if (command.size() >= 2)
            {
                int8_t offset = command[0] - static_cast<uint8_t>(Tiny::Drivers::Input::TITinyConCommands::MpuConfig1);
                if (offset >= 0 && offset < GamepadController::MaxMpuControllers)
                {
                    Controller.SetAccelerometerRange(offset, Tiny::Drivers::Input::TITinyConAccelerometerRanges(command[1] >> 4));
                    Controller.SetGyroscopeRange(offset, Tiny::Drivers::Input::TITinyConGyroscopeRanges(command[1] & 0xF));
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                    return true;
                }
                else
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidMpuIndex;
                    LogCommand::Error("Invalid MPU Index: ", offset, Tiny::TIEndl);
                    return false;
                }
            }

            break;
        case Tiny::Drivers::Input::TITinyConCommands::MPUDataEnable:
            if (command.size() >= 2)
            {
                Controller.SetAccelerationEnabled((command[1] & 0x08) != 0);
                Controller.SetAngularVelocityEnabled((command[1] & 0x04) != 0);
                Controller.SetOrientationEnabled((command[1] & 0x02) != 0);
                Controller.SetTemperatureEnabled((command[1] & 0x01) != 0);
                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                return true;
            }

            break;
        case Tiny::Drivers::Input::TITinyConCommands::FeatureEnable:
            if (command.size() >= 2)
            {
                SetI2CEnabled((command[1] & 0x04) != 0);
                SetBLEEnabled((command[1] & 0x02) != 0);
                SetUSBEnabled((command[1] & 0x01) != 0);
                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                return true;
            }

            break;
        default:
            break;
    }

    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
    return false;
}
