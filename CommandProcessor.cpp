#include "CommandProcessor.h"

using LogCommand = Tiny::TILogTarget<TinyCon::CommandLogLevel>;

void TinyCon::CommandProcessor::Init()
{
    // This ensures, that each register byte we don't write will read back the address of the register, providing a
    // good way to determine which registers are being read properly and where a read is potentially accessing the
    // wrong register.
    for (auto i = 0; i < MaxRegisters; ++i) Registers[i] = i & 0xFF;

    // Set the read-only registers with static data and initialize the others with the defaults
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::ID, Controller.Id);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Version, 0, Tiny::Drivers::Input::TITinyConVersion >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Version, 1, Tiny::Drivers::Input::TITinyConVersion & 0xFF);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Magic, 0, Tiny::Drivers::Input::TITinyConMagic >> 8);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::Magic, 1, Tiny::Drivers::Input::TITinyConMagic & 0xFF);

    // Initialize the configurable registers with the default settings
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::MPUDataEnable,
                Controller.GetAccelerationEnabled() << 3 | Controller.GetAngularVelocityEnabled() << 2 |
                Controller.GetOrientationEnabled() << 1 | Controller.GetTemperatureEnabled());
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::FeatureEnable, GetI2CEnabled() << 2 | GetBLEEnabled() << 1 | GetUSBEnabled());

    for (int8_t i = 0; i < GamepadController::MaxMpuControllers; ++i)
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::MpuConfig1, i,
                    static_cast<uint8_t>(Controller.GetAccelerometerRange(i)) << 4 | static_cast<uint8_t>(Controller.GetGyroscopeRange(i)));

    // Initialize the dynamic data
    Update();
}

void TinyCon::CommandProcessor::Update()
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
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::HapticTypes, i, static_cast<uint8_t>(Controller.GetHapticType(i)));
    for (int8_t i = 0; i < GamepadController::MaxInputControllers; ++i)
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::ControllerTypes, i, static_cast<uint8_t>(Controller.GetControllerType(i)));
    for (int8_t i = 0; i < GamepadController::MaxMpuControllers; ++i)
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::MpuTypes, i, static_cast<uint8_t>(Controller.GetMpuType(i)));

    SetRegister(Tiny::Drivers::Input::TITinyConCommands::AxisCount, Controller.GetAxisCount());
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::ButtonCount, Controller.GetButtonCount());
    constexpr auto dataStart = static_cast<uint8_t>(Tiny::Drivers::Input::TITinyConCommands::Data);
    auto dataOffset = Controller.MakeMpuBuffer({Registers.data() + dataStart, Registers.size() - dataStart});

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
    if (Controller.GetButtonCount() & 7) SetRegister(Tiny::Drivers::Input::TITinyConCommands::Data, dataOffset++, value);
    for (auto i = 0; i < Controller.GetAxisCount(); ++i)
    {
        auto axis = Tiny::Math::HalfFromFloat(Controller.GetAxis(i));
        if ((axis >= 0x7c00 && axis < 0x8000) || (axis >= 0xfc00)) axis = 0x0000;
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::Data, dataOffset++, axis >> 8);
        SetRegister(Tiny::Drivers::Input::TITinyConCommands::Data, dataOffset++, axis & 0xFF);
    }

    for (dataOffset += dataStart; dataOffset < Registers.size(); ++dataOffset) Registers[dataOffset] = dataOffset & 0xFF;
}

bool TinyCon::CommandProcessor::ProcessCommand(Tiny::Collections::TIFixedSpan<uint8_t> command)
{
    LogCommand::Verbose("CMD(", command.size(), "): ");
    for (std::size_t i = 0; i < command.size(); ++i)
    {
        if (command[i] < 0x10)  LogCommand::Verbose("0");
        LogCommand::Verbose(command[i], Tiny::TIFormat::Hex, " ");
    }
    LogCommand::Verbose(Tiny::TIEndl);

    LastParameter = {};
    const uint8_t reg = command[0];
    switch (Tiny::Drivers::Input::TITinyConCommands(reg))
    {
        case Tiny::Drivers::Input::TITinyConCommands::ID:
            if (command.size() > 1)
            {
                Controller.Id = command[1];
                Registers[reg] = command[1];

                LastParameter = {command[1]};
                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                LogCommand::Debug("ID:", command[1], Tiny::TIEndl);
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::Reset:
            if (command.size() > 1)
            {
                if (command[1] == Tiny::Drivers::Input::TITinyConResetConfirm)
                {
                    I2CEnabled = true;
                    USBEnabled = TinyConUSBEnabledByDefault;
                    BLEEnabled = TinyConBLEEnabledByDefault;
                    Controller.Reset();
                    Init();
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                    LogCommand::Debug("RST", Tiny::TIEndl);
                }
                else
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidResetValue;
                    LogCommand::Error("EIRV:", command[1], Tiny::TIEndl);
                }
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig1:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig2:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig3:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig4:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig5:
        case Tiny::Drivers::Input::TITinyConCommands::MpuConfig6:
            if (command.size() > 1)
            {
                int8_t offset = reg - static_cast<uint8_t>(Tiny::Drivers::Input::TITinyConCommands::MpuConfig1);
                if (offset >= 0 && offset < GamepadController::MaxMpuControllers)
                {
                    Controller.SetAccelerometerRange(offset, Tiny::Drivers::Input::TITinyConAccelerometerRanges(command[1] >> 4));
                    Controller.SetGyroscopeRange(offset, Tiny::Drivers::Input::TITinyConGyroscopeRanges(command[1] & 0xF));
                    Registers[reg] = command[1];

                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                    LogCommand::Debug("MPUCFG", offset, ":", command[1], Tiny::TIFormat::Hex, Tiny::TIEndl);
                }
                else
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidMpuIndex;
                    LogCommand::Error("IMI:", offset, Tiny::TIEndl);
                }

                LastParameter = {command[1]};
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::MPUDataEnable:
            if (command.size() > 1)
            {
                Controller.SetAccelerationEnabled((command[1] & 0x08) != 0);
                Controller.SetAngularVelocityEnabled((command[1] & 0x04) != 0);
                Controller.SetOrientationEnabled((command[1] & 0x02) != 0);
                Controller.SetTemperatureEnabled((command[1] & 0x01) != 0);
                Registers[reg] = command[1];

                LastParameter = {command[1]};
                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                LogCommand::Debug("MPUDE", ":", command[1], Tiny::TIFormat::Hex, Tiny::TIEndl);
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::FeatureEnable:
            if (command.size() > 1)
            {
                SetI2CEnabled((command[1] & 0x04) != 0);
                SetBLEEnabled((command[1] & 0x02) != 0);
                SetUSBEnabled((command[1] & 0x01) != 0);
                Registers[reg] = command[1];

                LastParameter = {command[1]};
                LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                LogCommand::Debug("FE:", command[1], Tiny::TIFormat::Hex, Tiny::TIEndl);
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::Haptic:
            if (command.size() > 13)
            {
                switch (Tiny::Drivers::Input::TITinyConHapticCommands(command[2]))
                {
                    case Tiny::Drivers::Input::TITinyConHapticCommands::PlayWaveform:
                    case Tiny::Drivers::Input::TITinyConHapticCommands::PlayRealtime:
                        if (command[3] > 8)
                        {
                            LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticDataSize;
                            LogCommand::Error("EIHDS:", command[2], Tiny::TIEndl);
                        }
                        else
                        {
                            LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                            for (auto bit = 0; bit < 8; ++bit)
                                if ((!Controller.GetHapticPresent(bit) || (bit >= GamepadController::MaxHapticControllers)) && (command[1] & (1 << bit)) != 0)
                                {
                                    // We'll still execute the command, but we'll return an error
                                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::WarningUnknownHapticController;
                                    LogCommand::Warning("WUHC:", bit, Tiny::TIEndl);
                                }

                            Controller.AddHapticCommand({command.data() + 1, command.size() - 1});
                            LastParameter = {command[1], command[2]};
                            LogCommand::Debug("HC", Tiny::TIEndl);
                        }
                        break;
                    default:
                        LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticCommand;
                        LogCommand::Error("EIH:", command[2], Tiny::TIEndl);
                        break;
                }
            }
            else if (command.size() > 2)
            {
                int8_t controller = command[1];
                int8_t queueIndex = command[2];
                if (controller >= GamepadController::MaxHapticControllers || !Controller.GetHapticPresent(controller))
                {
                    memset(Registers.data() + reg, 0xFF, 12);
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticController;
                    LogCommand::Error("EIHC:", command[1], Tiny::TIEndl);
                }
                else if (queueIndex >= 8)
                {
                    memset(Registers.data() + reg, 0xFF, 12);
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticDataIndex;
                    LogCommand::Error("EIHDI:", queueIndex, Tiny::TIEndl);
                }
                else
                {
                    Registers[reg] = static_cast<uint8_t>(Controller.GetHapticCommand(controller, queueIndex));
                    Registers[reg + 1] = Controller.GetHapticCommandCount(controller, queueIndex);
                    for (int8_t i = 0; i < 8; ++i) Registers[reg + 2 + i] = Controller.GetHapticCommandData(controller, queueIndex, i);
                    Registers[reg + 10] = Controller.GetHapticCommandDuration(controller, queueIndex) >> 8;
                    Registers[reg + 11] = Controller.GetHapticCommandDuration(controller, queueIndex) & 0xFF;
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                    LogCommand::Debug("HQ:", controller, ":", queueIndex, Tiny::TIEndl);
                }

                LastParameter = {command[1], command[2]};
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::HapticRemove:
            if (command.size() > 2)
            {
                int8_t controller = command[1];
                int8_t queueIndex = command[2];
                if (controller >= GamepadController::MaxHapticControllers || !Controller.GetHapticPresent(controller))
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticController;
                    LogCommand::Error("EIHC:", controller, Tiny::TIEndl);
                }
                else if (queueIndex >= 8)
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticDataIndex;
                    LogCommand::Error("EIHDI:", queueIndex, Tiny::TIEndl);
                }
                else
                {
                    Controller.RemoveHapticCommand(controller, queueIndex);
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                    LogCommand::Debug("HR:", controller, ":", queueIndex, Tiny::TIEndl);
                }

                LastParameter = {command[1], command[2]};
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::HapticQueueSize:
            // This and the haptic command are the only registers that require additional data
            // to be read. The haptic command does require 14 byte anyway, so this is fine,
            // but the queue size command needs this special case.
            if (command.size() > 1)
            {
                int8_t controller = command[1];
                if (controller >= GamepadController::MaxHapticControllers || !Controller.GetHapticPresent(controller))
                {
                    Registers[reg] = 0xFF;
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticController;
                    LogCommand::Error("EIHC:", controller, Tiny::TIEndl);
                }
                else
                {
                    Registers[reg] = Controller.GetHapticQueueSize(controller);
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                    LogCommand::Debug("HQS:", controller, Tiny::TIEndl);
                }

                LastParameter = {command[1]};
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        case Tiny::Drivers::Input::TITinyConCommands::HapticReset:
            if (command.size() > 1)
            {
                if (command[1] == Tiny::Drivers::Input::TITinyConHapticClearConfirm)
                {
                    Controller.ClearHapticCommands();
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
                    LogCommand::Debug("HC", Tiny::TIEndl);
                }
                else
                {
                    LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidHapticClearConfirm;
                    LogCommand::Error("EIHCC:", command[1], Tiny::TIEndl);
                }
            }
            else LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorIncompleteCommand;
            break;
        default:
            LastCommandStatus = Tiny::Drivers::Input::TITinyConCommandStatus::ErrorInvalidCommand;
            break;
    }

    // This needs to update every time we receive a byte, users could be reading back the status at any time
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 0, LastCommand);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 1, LastParameter[0]);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 2, LastParameter[1]);
    SetRegister(Tiny::Drivers::Input::TITinyConCommands::LastCommand, 3, static_cast<uint8_t>(LastCommandStatus));
    return LastCommandStatus == Tiny::Drivers::Input::TITinyConCommandStatus::Ok;
}
