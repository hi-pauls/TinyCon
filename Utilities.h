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
        *data++ = half & 0xFF;
        *data++ = (half >> 8) & 0xFF;
    }
}
