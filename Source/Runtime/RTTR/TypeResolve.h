#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>

#include <vector>
#include <array>
#include <string>
#include <type_traits>

namespace Horizon::Reflect
{
	template<typename T>
	constexpr TypeKind KindOf()
	{
		if constexpr (std::is_same_v<T, b8>)
			return TypeKind::Boolean;
		else if constexpr (std::is_same_v<T, c8>)
			return TypeKind::Char;
		else if constexpr (std::is_same_v<T, i8>)
			return TypeKind::Signed8;
		else if constexpr (std::is_same_v<T, i16>)
			return TypeKind::Signed16;
		else if constexpr (std::is_same_v<T, i32>)
			return TypeKind::Signed32;
		else if constexpr (std::is_same_v<T, i64>)
			return TypeKind::Signed64;
		else if constexpr (std::is_same_v<T, u8>)
			return TypeKind::Unsigned8;
		else if constexpr (std::is_same_v<T, u16>)
			return TypeKind::Unsigned16;
		else if constexpr (std::is_same_v<T, u32>)
			return TypeKind::Unsigned32;
		else if constexpr (std::is_same_v<T, u64>)
			return TypeKind::Unsigned64;
		else if constexpr (std::is_same_v<T, f32>)
			return TypeKind::Float32;
		else if constexpr (std::is_same_v<T, f64>)
			return TypeKind::Float64;
		else if constexpr (std::is_same_v<T, std::string>)
			return TypeKind::String;
		else if constexpr (std::is_enum_v<T>)
			return TypeKind::Enum;
		else
			return TypeKind::Object;
	}

	template<typename T>
	struct TypeResolve
	{
		using Element = T;
		static constexpr TypeMode Mode = TypeMode::Compose;
		static constexpr TypeKind Kind = KindOf<T>();
	};


	template<typename E, typename A>
	struct TypeResolve<std::vector<E, A>>
	{
		using Element = E;
		static constexpr TypeMode Mode = TypeMode::Array;
		static constexpr TypeKind Kind = KindOf<E>();
	};

	template<typename E, usize N>
	struct TypeResolve<std::array<E, N>>
	{
		using Element = E;
		static constexpr TypeMode Mode = TypeMode::Array;
		static constexpr TypeKind Kind = KindOf<E>();
	};
}