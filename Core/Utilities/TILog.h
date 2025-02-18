#pragma once

#include <Arduino.h>
#include <cstdint>

namespace Tiny
{
    enum class TILogLevel : uint8_t
    {
        None = 0,
        Error = 1,
        Warning = 2,
        Info = 3,
        Debug = 4,
        Verbose = 5
    };

    constexpr TILogLevel GlobalLogThreshold = TILogLevel::Debug;

    struct TIEndlType { static constexpr const char* Value = "\n"; };
    constexpr TIEndlType TIEndl = TIEndlType();
    template <typename ...TValues> inline void LogFunc(TValues... values) { Serial.print(values...); }
    template <> inline void LogFunc<TIEndlType>(TIEndlType) { LogFunc(TIEndlType::Value); }
    enum class TIFormat : uint8_t { Hex = HEX, Dec = DEC, Bin = BIN, Oct = OCT };

    template <TILogLevel CDefaultLevel>
    struct TILogTarget
    {
        template <typename ...TValues> static void Display(TValues... values) { Log<CDefaultLevel>(values...); }
        template <typename ...TValues> static void Error(TValues... values) { Log<TILogLevel::Error>(values...); }
        template <typename ...TValues> static void Warning(TValues... values) { Log<TILogLevel::Warning>(values...); }
        template <typename ...TValues> static void Info(TValues... values) { Log<TILogLevel::Info>(values...); }
        template <typename ...TValues> static void Debug(TValues... values) { Log<TILogLevel::Debug>(values...); }
        template <typename ...TValues> static void Verbose(TValues... values) { Log<TILogLevel::Verbose>(values...); }

    private:
        template <TILogLevel CLogLevel, typename TValue>
        static void Log(TValue value)
        {
            if constexpr (GlobalLogThreshold <= CDefaultLevel && GlobalLogThreshold <= CLogLevel) LogFunc(value);
        }
        template<TILogLevel CLogLevel, typename TValue>
        static void Log(TValue value, TIFormat format)
        {
            if constexpr (GlobalLogThreshold <= CDefaultLevel && GlobalLogThreshold <= CLogLevel) LogFunc(value, static_cast<int>(format));
        }
        template <TILogLevel CLogLevel, typename TValue, typename ...TValues>
        static void Log(TValue value, TValues... values)
        {
            Log<CLogLevel>(value);
            Log<CLogLevel>(values...);
        }
        template <TILogLevel CLogLevel, typename TValue, typename ...TValues>
        static void Log(TValue value, TIFormat format, TValues... values)
        {
            Log<CLogLevel>(value, format);
            Log<CLogLevel>(values...);
        }
    };
}
