#pragma once

#include <cstdint>

namespace Tiny::Math
{
    constexpr uint16_t HalfFromFloat(float value)
    {
        union { float f; uint32_t i; } bits = {value};
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

    template <typename TLhs, typename TRhs>
    constexpr auto Min(const TLhs& a, const TRhs& b) { return a < b ? a : b; }
    template <typename TLhs, typename TRhs>
    constexpr auto Max(const TLhs& a, const TRhs& b) { return a > b ? a : b; }
}
