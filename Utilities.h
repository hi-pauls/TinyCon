#pragma once

#include "Config.h"

#include <cstdint>

/**
 * We are not building this into a die, so we can go dynamic on registers, potentially sharing with the driver implementation.
 *
 * Register Map:
 * 0x00        No Op
 * 0x01        ID            0x5A
 * 0x02        Version       1
 * 0x03        Controller 1  Type (0: None, 1: Seasaw)
 * 0x04                      [7-5:Axis Count(2)|4-0:Button Count(5)]
 * 0x05                      MPU Type (0: None, 1: ICM20948)
 * 0x06                      Haptic Type (0: None, 1: DRV2605L)
 * 0x07        Reserved
 * 0x08        Controller 2  Type (0: None, 1: Seasaw)
 * 0x09                      [7-5:Axis Count(2)|4-0:Button Count(5)]
 * 0x0A                      MPU Type (0: None, 1: ICM20948)
 * 0x0B                      Haptic Type (0: None, 1: DRV2605L)
 * 0x0C        Reserved
 * 0x0D        LastCommand   Command
 * 0x0E                      Param
 * 0x0F                      Status
 * 0x10 - 0x11 Battery       Percentage
 * 0x12 - 0x13               Voltage
 * 0x14 - 0x15               Temperature
 * 0x16 - 0x17 Reserved
 * 0x18        Device Enable Controller 1 [7-6:Reserved|5:Accel|4:Gyro|3:Mag|2:Temp|1:Input|0:Drv]
 * 0x19                      Reserved
 * 0x1A                      Controller 2 [7-6:Reserved|5:Accel|4:Gyro|3:Mag|2:Temp|1:Input|0:Drv]
 * 0x1B                      Reserved
 * 0x1C        Features      Enabled [7-3:Reserved|2:I2C|1:BLE|0:USB]
 * 0x1D                      Reserved
 * 0x1E        MPU           Settings [7-4:Accel|3-0:Gyro]
 * 0x1F                      Reserved
 * 0x20        Haptic        Haptic Controller
 * 0x21                      Haptic Command or Slot Index
 * 0x22                      Haptic Count
 * 0x23 - 0x2A               Haptic Data
 * 0x2B - 0x2C               Haptic Duration
 * 0x2D                      QueueSize
 * 0x2E                      Remove the command at the given index
 * 0x2F                      Reset
 * 0x30        Read Data     Up to 82 Byte
 * 0x40 - 0x91 Data
 *             MPU1 Accel 6 Byte (if present and enabled)
 *                  Gyro  6 Byte (if present and enabled)
 *                  Mag   6 Byte (if present and enabled)
 *                  Temp  2 Byte (if present and enabled)
 *             MPU2 Accel 6 Byte (if present and enabled)
 *                  Gyro  6 Byte (if present and enabled)
 *                  Mag   6 Byte (if present and enabled)
 *                  Temp  2 Byte (if present and enabled)
 *             Buttons1 1-4 Byte (Full bytes, depending on button count, always present)
 *             Buttons2 1-4 Byte (Full bytes, depending on button count, always present)
 *             Axis1 0-8 * 2 Byte (depending on axis count)
 *             Axis2 0-8 * 2 Byte (depending on axis count)
 */
enum class Commands : uint8_t
{
    ID = 0x1,
    Version = 0x2,
    Controller1 = 0x3, // 4
    Controller2 = 0x8, // 4
    LastCommand = 0xD, // 3
    BatteryPercentage = 0x10, // 2
    BatteryVoltage = 0x12, // 2
    BatteryTemperature = 0x14, // 2
    Controller1DeviceEnable = 0x18,
    Controller2DeviceEnable = 0x1A,
    FeatureEnable = 0x1C,
    MpuSettings = 0x1E,
    Haptic = 0x20,
    HapticQueueSize = 0x2D,
    HapticRemove = 0x2E,
    HapticReset = 0x2F,
    ReadData = 0x30 // Up to 82 Byte
};
constexpr uint8_t CommandOrder[] = {0x1, 0x2, 0x3, 0x8, 0xD, 0x10, 0x12, 0x14, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x2D, 0x2E, 0x2F, 0x40};
constexpr uint8_t CommandId(Commands command) { return static_cast<uint8_t>(command); }
constexpr Commands Command(uint8_t command) { return static_cast<Commands>(command); }

struct Span { const uint8_t* Data; std::size_t Size; Span(const uint8_t* data, std::size_t size) : Data(data), Size(size) {} };
struct Vector2 { float X, Y; };
struct Vector3 { float X, Y, Z; };

constexpr uint16_t FloatToHalf(float value)
{
    union { float f; uint32_t i; } bits = { value };
    uint16_t sign = (bits.i >> 16) & 0x8000;
    uint16_t exponent = ((bits.i >> 23) & 0xFF) - 127 + 15;
    uint16_t mantissa = (bits.i >> 13) & 0x3FF;

    if (exponent <= 0)
    {
        exponent = 0;
        mantissa = 0;
    }
    else if (exponent >= 31)
    {
        exponent = 31;
        mantissa = 0;
    }

    return sign | (exponent << 10) | mantissa;
}

inline void FillHalf(uint8_t*& data, float value)
{
    uint16_t half = FloatToHalf(value);
    *data++ = half & 0xFF;
    *data++ = (half >> 8) & 0xFF;
}

