// BitwiseOperators.h

#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <type_traits>

namespace Horizon
{
	template<typename T>
		requires std::is_enum_v<T>
	constexpr T operator|(T lhs, T rhs)
	{
		using Underlying = std::underlying_type_t<T>;
		return static_cast<T>(static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs));
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr T& operator|=(T& lhs, T rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr T operator&(T lhs, T rhs)
	{
		using Underlying = std::underlying_type_t<T>;
		return static_cast<T>(static_cast<Underlying>(lhs) & static_cast<Underlying>(rhs));
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr T& operator&=(T& lhs, T rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr T operator^(T lhs, T rhs)
	{
		using Underlying = std::underlying_type_t<T>;
		return static_cast<T>(static_cast<Underlying>(lhs) ^ static_cast<Underlying>(rhs));
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr T& operator^=(T& lhs, T rhs)
	{
		lhs = lhs ^ rhs;
		return lhs;
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr T operator~(T value)
	{
		using Underlying = std::underlying_type_t<T>;
		return static_cast<T>(~static_cast<Underlying>(value));
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr b8 HasFlag(T value, T mask)
	{
		using Underlying = std::underlying_type_t<T>;
		return (static_cast<Underlying>(value) & static_cast<Underlying>(mask)) != Underlying(0);
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr b8 HasAllFlags(T value, T mask)
	{
		using Underlying = std::underlying_type_t<T>;
		return (static_cast<Underlying>(value) & static_cast<Underlying>(mask)) == static_cast<Underlying>(mask);
	}

	template<typename T>
		requires std::is_enum_v<T>
	constexpr b8 IsSameFlag(T lhs, T rhs) { return lhs == rhs; }

	template<typename T>
		requires std::is_enum_v<T>
	constexpr b8 IsAnyFlag(T value)
	{
		using Underlying = std::underlying_type_t<T>;
		return static_cast<Underlying>(value) != Underlying(0);
	}
}