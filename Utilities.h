#pragma once

#include "Config.h"

#include "Core/Math/TIMath.h"

#include <cstdint>

namespace TinyCon
{
    static constexpr int8_t NC = -1;
    enum class ActiveState : uint8_t { Low = 0, High };

    inline void FillHalf(uint8_t *&data, float value)
    {
        uint16_t half = Tiny::Math::HalfFromFloat(value);
        if ((half >= 0x7c00 && half < 0x8000) || (half >= 0xfc00)) half = 0x0000;
        *data++ = half & 0xFF;
        *data++ = (half >> 8) & 0xFF;
    }
}
