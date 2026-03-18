#pragma once

#include <type_traits>

namespace Horizon
{
    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, T>
    operator|(T lhs, T rhs)
    {
        using Underlying = typename std::underlying_type_t<T>;
        return static_cast<T>(static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs));
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, T&>
        operator|=(T& lhs, T rhs)
    {
        using Underlying = std::underlying_type_t<T>;
        lhs = static_cast<T>(static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs));
        return lhs;
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, T>
        operator&(T lhs, T rhs) {
        using Underlying = typename std::underlying_type_t<T>;
        return static_cast<T>(static_cast<Underlying>(lhs) & static_cast<Underlying>(rhs));
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, T&>
        operator&=(T& lhs, T rhs)
    {
        using Underlying = std::underlying_type_t<T>;
        lhs = static_cast<T>(static_cast<Underlying>(lhs) & static_cast<Underlying>(static_cast<Underlying>(rhs)));
        return lhs;
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, T>
        operator~(T lhs)
    {
        using Underlying = std::underlying_type_t<T>;
        return static_cast<T>(~static_cast<Underlying>(lhs));
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, b8>
        HasFlag(T lhs, T rhs)
    {
        using Underlying = typename std::underlying_type_t<T>;
        return (static_cast<Underlying>(lhs) & static_cast<Underlying>(rhs)) != 0;
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, b8>
        IsSameFlag(T lhs, T rhs)
    {
        using Underlying = typename std::underlying_type_t<T>;
        return (static_cast<Underlying>(lhs) == static_cast<Underlying>(rhs));
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, b8>
        operator==(T lhs, T rhs)
    {
        using Underlying = typename std::underlying_type_t<T>;
        return (static_cast<Underlying>(lhs) == static_cast<Underlying>(rhs));
    }

    template<typename T>
    constexpr std::enable_if_t<std::is_enum_v<T>, b8>
        operator!=(T lhs, T rhs)
    {
        using Underlying = typename std::underlying_type_t<T>;
        return (static_cast<Underlying>(lhs) != static_cast<Underlying>(rhs));
    }
}
