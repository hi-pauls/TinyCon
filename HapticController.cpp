#include "HapticController.h"
#include <cstring>
#include <memory>

namespace
{
    template <typename TWire>
    void WriteRegister(TWire& wire, uint8_t address, uint8_t reg, uint8_t value)
    {
        wire.beginTransmission(address);
        wire.write(reg);
        wire.write(value);
        wire.endTransmission();
    }

    template <typename TWire>
    void SetMode(TWire& wire, uint8_t address, uint8_t mode)
    {
        // 0x01, Mode = 0x00 for lib, 0x05 for realtime
        // 0x1D, 0x04 for Waveform, 0x0E for realtime, 0x1 is open loop, should be false
        WriteRegister(wire, address, 0x01, mode);
        if (mode == DRV2605Controller::DRV2605_MODE_INTTRIG) WriteRegister(wire, address, 0x1D, 0x00);
        else WriteRegister(wire, address, 0x1D, 0x0A);
    }

    template <typename TWire>
    void InitDRV2605(TWire& wire, uint8_t mode)
    {
        SetMode(wire, 0x5A, mode);
        // 0x02, Realtime
        // 0x03, Libraries = 0x6 or 0x16 for high impedance
        WriteRegister(wire, 0x5A, 0x03, 0x06);
        // 0x17 for open loop, 0x16 closed loop, Voltage 0x4F
        WriteRegister(wire, 0x5A, 0x16, 0x1F);
        // 0x1A, 0x80 for LRA, 0x0 for ERM
        WriteRegister(wire, 0x5A, 0x1A, 0x80);
        // 0x22, Resonance 0x30
        WriteRegister(wire, 0x5A, 0x22, 0x30);
    }
}

void DRV2605Controller::Init(TwoWire& wire)
{
    SoftwareMode = false;
    I2C.Hardware = &wire;
    Init();
}

void DRV2605Controller::Init(SoftWire& wire)
{
    SoftwareMode = true;
    I2C.Software = &wire;
    Init();
}

void DRV2605Controller::Init()
{
    if (SoftwareMode)
    {
        I2C.Software->beginTransmission(0x5A);
        Present = I2C.Software->endTransmission() == 0;
    }
    else
    {
        I2C.Hardware->beginTransmission(0x5A);
        Present = I2C.Hardware->endTransmission() == 0;
    }

    if (Present)
    {
        Mode = DRV2605_MODE_INTTRIG;
        if (SoftwareMode) InitDRV2605(*I2C.Software, Mode);
        else InitDRV2605(*I2C.Hardware, Mode);
    }
}

void DRV2605Controller::PlayRealtime(uint8_t value)
{
    if (!Present) LOG_CONTROLLER(", No DRV2605");

    LOG_CONTROLLER(", Realtime: "); LOG_CONTROLLER(value);
    if (Mode != DRV2605_MODE_REALTIME)
    {
        Mode = DRV2605_MODE_REALTIME;
        if (SoftwareMode) SetMode(*I2C.Software, 0x5A, Mode);
        else SetMode(*I2C.Hardware, 0x5A, Mode);
    }

    if (SoftwareMode) WriteRegister(*I2C.Software, 0x5A, 0x02, value);
    else WriteRegister(*I2C.Hardware, 0x5A, 0x02, value);
}

void DRV2605Controller::PlayWaveform(const uint8_t* data)
{
    if (!Present) LOG_CONTROLLER(", No DRV2605");

    LOG_CONTROLLER(", Waveforms: ");
    for (int8_t i = 0; i < 8; ++i) if (auto d = data[i]) { if (i) LOG_CONTROLLER(", "); LOG_CONTROLLER(d); d++; }

    if (Mode != DRV2605_MODE_INTTRIG)
    {
        Mode = DRV2605_MODE_INTTRIG;
        if (SoftwareMode) SetMode(*I2C.Software, 0x5A, Mode);
        else SetMode(*I2C.Hardware, 0x5A, Mode);
    }

    // 0x04 - 0x0B, Sequence
    for (int8_t i = 0; i < 8; ++i)
    {
        auto value = data[i];
        if (SoftwareMode) WriteRegister(*I2C.Software, 0x5A, 0x04 + i, value);
        else WriteRegister(*I2C.Hardware, 0x5A, 0x04 + i, value);
        if (!value)
            // If we have less than 8 values, we can stop
            // here, we set the stop value already.
            break;
    }

    // 0x0C Go, 0x01 Go
    if (SoftwareMode) WriteRegister(*I2C.Software, 0x5A, 0x0C, 0x01);
    else WriteRegister(*I2C.Hardware, 0x5A, 0x0C, 0x01);
}

void DRV2605Controller::Stop()
{
    // 0x0C Go, 0x00 Stop
    if (SoftwareMode)
    {
        if (Mode == DRV2605_MODE_REALTIME) WriteRegister(*I2C.Software, 0x5A, 0x02, 0x0);
        else WriteRegister(*I2C.Software, 0x5A, 0x0C, 0x0);
    }
    else
    {
        if (Mode == DRV2605_MODE_REALTIME) WriteRegister(*I2C.Hardware, 0x5A, 0x02, 0x0);
        else WriteRegister(*I2C.Hardware, 0x5A, 0x0C, 0x0);
    }
}

void HapticController::Init(TwoWire& wire)
{
    DRV2605.Init(wire);
    Present = DRV2605.Present;
}

void HapticController::Init(SoftWire& wire)
{
    DRV2605.Init(wire);
    Present = DRV2605.Present;
}

void HapticController::Insert(uint8_t command, uint8_t count, const uint8_t* data, uint16_t duration)
{
    Commands[Head].Command = static_cast<HapticCommands>(command);
    Commands[Head].Count = count;
    std::memcpy(Commands[Head].Value, data, count);
    if (count < 8) Commands[Head].Value[count] = 0;
    Commands[Head].Duration = duration;
    Head = (Head + 1) & 7;
    if (Head == Tail)
        // Moves the tail further, we queued more than the length of the
        // buffer, so we skip the old commands in favour of the new ones.
        Tail = (Tail + 1) & 7;
}

void HapticController::Update(int32_t deltaTime)
{
    if (!Present && Enabled)
    {
        LOG_CONTROLLER(", Trying Init");
        DRV2605.Init(Wire);
        if ((Present = DRV2605.Present)) LOG_CONTROLLER(", Success");
        else LOG_CONTROLLER(", Failed");
    }

    if (HasNewCommand(deltaTime))
    {
        LOG_CONTROLLER(", Play");
        const auto& command = Commands[Tail];
        switch (command.Command)
        {
            case HapticCommands::PlayWaveform:
                DRV2605.PlayWaveform(&command.Value[0]);
                break;
            case HapticCommands::PlayRealtime:
                DRV2605.PlayRealtime(command.Value[RealtimeIndex]);
                break;
            default:
                DRV2605.Stop();
                break;
        }
    }
}

bool HapticController::HasNewCommand(int32_t deltaTime)
{
    if (Tail == Head) { LOG_CONTROLLER(", Finished"); return false; }

    auto& command = Commands[Tail];
    LOG_CONTROLLER(", Command: "); LOG_CONTROLLER(HapticCommandId(command.Command)); LOG_CONTROLLER(", Duration: "); LOG_CONTROLLER(command.Duration);
    if (command.Command == HapticCommands::PlayRealtime)
    {
        if (RealtimeTimeLeft < deltaTime)
        {
            if (++RealtimeIndex < command.Count)
            {
                RealtimeTimeLeft = RealtimePerCommandDuration - (deltaTime - RealtimeTimeLeft);
                LOG_CONTROLLER(", Next");
                return true;
            }
            // Assume we also at this point have command.Duration being <= than deltaTime,
            // causing this command to reject.
        }
        else
            // So at this point we know we just need to count down the time and
            // command.Duration > deltaTime, so we'll end up returning false later.
            RealtimeTimeLeft -= deltaTime;
    }

    if (command.Duration > deltaTime)
    {
        command.Duration -= deltaTime;
        LOG_CONTROLLER(", Left: "); LOG_CONTROLLER(command.Duration);
        return false;
    }

    do
    {
        LOG_CONTROLLER(", Next");
        deltaTime -= command.Duration;
        Tail = (Tail + 1) & 7;
        command = Commands[Tail];
        RealtimeIndex = 0;
        RealtimePerCommandDuration = command.Duration / command.Count;
        RealtimeTimeLeft = RealtimePerCommandDuration;
    }
    while (command.Duration < deltaTime && Tail != Head);
    return Tail != Head;
}

void HapticController::RemoveHapticCommand(int8_t index)
{
    index = GetCommandIndex(index);
    Commands[index].Command = HapticCommands::Noop;
    Commands[index].Count = 0;
    Commands[index].Duration = 0;
}
