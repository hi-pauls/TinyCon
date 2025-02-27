#include "I2C.h"

#include "HapticController.h"

using LogI2C = Tiny::TILogTarget<TinyCon::I2CLogLevel>;

std::function<void(int)> TinyCon::I2CController::I2CReceiveCallback = [](int) {};
std::function<void()> TinyCon::I2CController::I2CRequestCallback = []() {};
void TinyCon::I2CController::I2CSlaveReceive(int count) { I2CReceiveCallback(count); }
void TinyCon::I2CController::I2CSlaveRequest() { I2CRequestCallback(); }

void TinyCon::I2CController::Init()
{
    I2CReceiveCallback = [this](int count) { Receive(); };
    I2CRequestCallback = [this]() { Send(); };
    SlaveI2C.onRequest(I2CSlaveRequest);
    SlaveI2C.onReceive(I2CSlaveReceive);
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
    if (Processor.GetI2CEnabled())
    {
        LogI2C::Debug("IR:");
        if (RegisterAddress < 0x10) LogI2C::Debug("0");
        LogI2C::Debug(RegisterAddress, Tiny::TIFormat::Hex, Tiny::TIEndl);
        SlaveI2C.write(Processor.Registers.data() + RegisterAddress, TinyCon::MaxI2CWriteBufferFill);
    }
}

void TinyCon::I2CController::Receive()
{
    if (Processor.GetI2CEnabled())
    {
        std::array<uint8_t, CommandProcessor::MaxCommandSize> buffer = {};
        auto size = Tiny::Math::Min(buffer.size(), static_cast<uint8_t>(SlaveI2C.available()));
        if (size > 0)
        {
            SlaveI2C.readBytes(buffer.data(), size);
            RegisterAddress = buffer[0];

            LogI2C::Verbose("IW:");
            for (std::size_t i = 0; i < size; ++i)
            {
                if (buffer[i] < 0x10) LogI2C::Verbose("0");
                LogI2C::Verbose(buffer[i], Tiny::TIFormat::Hex, " ");
            }
            LogI2C::Verbose(Tiny::TIEndl);

            if (size > 1) Processor.ProcessCommand({buffer.data(), size});
        }
    }
}