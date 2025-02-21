#pragma once

#include <cstdint>

namespace Tiny::Drivers::Input
{
    /**
     * This is the command or register map for the busses on the controller. It can be used by
     * the command report of the USB part, the HapticService of the BLE part or the I2C port.
     * Depending on the maximum number of bytes per I2C transaction, it may be necessary
     * to perform multiple reads. Due to limitations of the Arduino Wire library in slave mode
     * and the fact, that we can't detect stop-conditions, commands will always have to be sent
     * immediately before reading, otherwise the data returned is non-deterministic.
     */
    enum class TITinyConCommands : uint8_t
    {
        NoOp = 0x0,
        /** Modifiable controller ID */
        ID = 0x1,
        /** Version of the controller */
        Version = 0x2,
        /** VIN voltage, 2 bytes, half float, read-only */
        VinVoltage = 0x4,
        /** Battery charge percentage, 2 bytes, half float, read-only */
        BatteryPercentage = 0x6,
        /** Battery voltage, 2 bytes, half float, read-only */
        BatteryVoltage = 0x8,
        /** Battery temperature, 2 bytes, half float, read-only */
        BatteryTemperature = 0xA,
        /**
         * Last command, that executed, 4 bytes, read-only
         * 0: Command
         * 1-2: Params
         * 3: Status
         */
        LastCommand = 0xC,

        /**
         * Haptic controller command, this has 10 parameters when written, when reading, it requires one parameter for the
         * controller index to then allow reading the currently running command, or a second parameter for the slot index
         * to read the command at that index in the queue.
         * 0: Haptic Controller Index
         * 1: Haptic Command or Slot Index, commands are 0x01 for library waveforms and 0x02 for realtime data
         * 2: Number of values in the data section, the length of the data section does not change!
         * 3-10: The data
         * 11-12: Duration in ms for playback of the whole sequence.
         */
        Haptic = 0x10,
        /**
         * Haptic queue size, 1 byte. This has one additional parameter to indicate the controller to queue.
         * 0: Haptic Controller Mask (not index!)
         * 1: Haptic Command
         * 2: Number of values in the data section
         * 3-10: The data
         * 11-12: Duration in ms for playback of the whole sequence.
         */
        HapticQueueSize = 0x1D,
        /**
         * Remove the command at the given index, 0 byte. This has two additional parameters to indicate the controller to
         * remove the command from and the index of the command to remove. Write-only
         * 0: Haptic Controller Index
         * 1: Haptic Command Index
         */
        HapticRemove = 0x1E,
        /**
         * Reset the haptic controllers queue, 0 byte. This stops all haptic playback and clears the queue. Write-only
         */
        HapticReset = 0x1F,
        /**
         * Haptic driver types, 1 byte, up to 8
         * 0: Haptic 1 Type (0: None, 1: DRV2605L)
         */
        HapticTypes = 0x20,
        /**
         * Controller Types, 8x 1 byte
         * 0: Controller 1 Type (0: None, 1: Seesaw, 2: Pins)
         */
        ControllerTypes = 0x28,
        /**
         * MPU availability mask, 1 byte, up to 6
         * 0: MPU Type (0: None, 1: ICM20948)
         */
        // XXX: If we can change this to support more than 6 MPUs, we could support MPU-based MOCAP-rigs
        MpuTypes = 0x30,
        /**
         * MPU settings, 1 byte per MPU, up to 6
         * 0: [7-4:Accel|3-0:Gyro]
         */
        MpuConfig1 = 0x36,
        MpuConfig2 = 0x37,
        MpuConfig3 = 0x38,
        MpuConfig4 = 0x39,
        MpuConfig5 = 0x3A,
        MpuConfig6 = 0x3B,
        /** Total Axis Count */
        AxisCount = 0x3C,
        /** Total Button Count */
        ButtonCount = 0x3D,
        /**
         * MPU features enable, 1 byte, read-write
         * 0: [7-4:Reserved|3:AccelEn|2:GyroEn|1:MagEn|0:TempEn]
         */
        MPUDataEnable = 0x3E,
        /**
         * Non-controller feature switches, 2 bytes, read-write
         * 0: [7-3:Reserved|2:I2C|1:BLE|0:USB]
         */
        FeatureEnable = 0x3F,

        /** Magic number for the controller, 2 bytes, 0x5443 or TC read-only */
        Magic = 0x40,

        /**
         * The controller data, containing all enabled MPUs before all buttons before all axis. If any of these are not
         * present, they will be skipped. A current full read will be 20 * 2 MPU bytes + 1 * 2 button bytes + 2 * 2 * 2 axis
         * bytes = 50 bytes in total. The maximum size is 20 * 8 MPU bytes + 4 * 8 button bytes + 2 * 8 * 8 axis bytes = 320
         * bytes. Since this does not fit our 256 - 64 = 192 byte address space, we currently only support a subset of the
         * available sensors. Reading requires one additional parameter for the page to read. A page is 192 bytes of
         * controller data. The data is paged instead of organized by controller to make reads more efficient. The maximum
         * number of pages is currently 2, starting with 0, as we only have 320 bytes of data at maximum. Read-only
         *     MPUs up to 6 * 20 byte in half-float format
         *          Accel 6 byte in half-float format, if enabled
         *          Gyro  6 byte in half-float format, if enabled
         *          Mag   6 byte in half-float format, if enabled
         *          Temp  2 byte in half-float format, if enabled
         *     Buttons 0-32 byte in boolean array format with up to 256 buttons
         *     Axis 0-64 * 2 byte in half-float format
         */
        Data = 0x42
    };

    static constexpr uint16_t TITinyConVersion = 1;
    static constexpr uint16_t TITinyConMagic = 0x5443;

    enum class TITinyConCommandStatus : uint8_t
    {
        Ok = 0,
        ErrorInvalidCommand,
        ErrorIncompleteCommand,
        ErrorInvalidMpuIndex,
        ErrorInvalidHapticDataSize,
        ErrorInvalidHapticController,
        ErrorInvalidHapticDataIndex,
        WarningUnknownHapticController
    };

    static constexpr bool IsOk(TITinyConCommandStatus status) { return status == TITinyConCommandStatus::Ok; }
    static constexpr bool IsError(TITinyConCommandStatus status) { return status > TITinyConCommandStatus::Ok && status < TITinyConCommandStatus::WarningUnknownHapticController; }

    enum class TITinyConMpuTypes : uint8_t
    {
        None = 0,
        ICM20948
    };

    enum class TITinyConAccelerometerRanges : uint8_t
    {
        Invalid = 0,
        G2,
        G4,
        G8,
        G16
    };

    enum class TITinyConGyroscopeRanges : uint8_t
    {
        Invalid = 0,
        D250,
        D500,
        D1000,
        D2000,
        D4000
    };

    enum class TITinyConHapticTypes : uint8_t
    {
        None = 0,
        DRV2605
    };

    enum class TITinyConHapticCommands
    {
        Noop = 0,
        PlayWaveform = 0x1,
        PlayRealtime = 0x2
    };

    enum class TITinyConControllerTypes : uint8_t
    {
        None = 0,
        Seesaw,
        Pins
    };
}