#pragma once

#include <cstdint>

namespace Tiny::Math
{
    template <typename TAxis, int8_t CDimensions>
    struct TIVector { TAxis Data[CDimensions]; };
    template <typename TAxis>
    struct TIVector<TAxis, 3> { union { struct { TAxis X, Y, Z; }; TAxis Data[3]; }; };
    using TIVector3F = TIVector<float, 3>;
}