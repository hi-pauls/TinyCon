#pragma once

#include <cstdint>

namespace Tiny::Collections
{
    template <typename TElement>
    class TIFixedSpan
    {
    public:
        TIFixedSpan(const uint8_t *data, std::size_t size) : Data(data), Size(size) {}

        const TElement& operator[](std::size_t index) { return Data[index]; }

        const TElement* data() const { return Data; }
        std::size_t size() const { return Size; }

    private:
        const TElement* Data;
        std::size_t Size;
    };
}