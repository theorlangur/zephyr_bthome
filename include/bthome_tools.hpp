#ifndef BTHHOME_TOOLS_HPP_
#define BTHHOME_TOOLS_HPP_

#include <cstddef>
#include <cstdint>

namespace BTHome
{
    enum class Flags: uint8_t
    {
        None = 0,
        Encryption = 1 << 0,
        Trigger = 1 << 2,
    };
    static const constexpr uint8_t kBTHomeVer = 2 << 5;
    constexpr Flags operator|(Flags f1, Flags f2) { return static_cast<Flags>(static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2)); }
    constexpr uint8_t operator|(Flags f1, uint8_t f2) { return static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2); }

    namespace tools
    {
        template<size_t Nth, class X, class... T>
        struct DataOffset;

        template<size_t Nth, class X, class... T>
            struct DataOffset<Nth, X, X, T...>
            {
                static constexpr uint8_t kOffset = X::kDataSize + 1 + DataOffset<Nth - 1, X, T...>::kOffset;
                static constexpr bool kFound = DataOffset<Nth - 1, X, T...>::kFound;
            };

        template<class X, class... T>
            struct DataOffset<1, X, X, T...>
            {
                static constexpr uint8_t kOffset = 0;
                static constexpr bool kFound = true;
            };

        template<size_t Nth, class X, class Y, class... T>
            struct DataOffset<Nth, X, Y, T...>
            {
                static constexpr uint8_t kOffset = Y::kDataSize + 1 + DataOffset<Nth, X, T...>::kOffset;
                static constexpr bool kFound = DataOffset<Nth, X, T...>::kFound;
            };

        template<size_t Nth, class X>
            struct DataOffset<Nth, X>
            {
                static constexpr uint8_t kOffset = 0xff;
                static constexpr bool kFound = false;
            };

    }
}
#endif
