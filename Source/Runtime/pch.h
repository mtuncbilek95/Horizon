#pragma once

#include <cstdint>
#include <limits>

namespace Horizon
{
	typedef char c8;
	typedef int8_t i8;
	typedef int16_t i16;
	typedef int32_t i32;
	typedef int64_t i64;
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;

	typedef size_t usize;

	typedef float f32;
	typedef double f64;

	typedef bool b8;

	inline constexpr u64 u64_max = std::numeric_limits<u64>::max();
	inline constexpr u32 u32_max = std::numeric_limits<u32>::max();
	inline constexpr u16 u16_max = std::numeric_limits<u16>::max();
	inline constexpr u8 u8_max = std::numeric_limits<u8>::max();

	inline constexpr u32 kInvalid32 = ~0u;
	inline constexpr u64 kInvalid64 = ~0ull;

	using HandleId = u64;
	inline constexpr HandleId InvalidHandleId = ~0ull;
	inline constexpr HandleId IndexBits = 32;
	inline constexpr HandleId IndexMask = (HandleId(1) << IndexBits) - 1;

	template<typename Tag>
	struct Handle
	{
		HandleId id = InvalidHandleId;

		bool IsValid() const { return id != InvalidHandleId; }

		explicit operator bool() const { return IsValid(); }

		bool operator==(const Handle& other) const { return id == other.id; }
		bool operator!=(const Handle& other) const { return id != other.id; }

		static Handle Generate(u32 index, u32 generation)
		{
			Handle hndl;
			hndl.id = (HandleId(generation) << IndexBits) | HandleId(index);
			return hndl;
		}

		u32 Index() const { return u32(id & IndexMask); }
		u32 Generation() const { return u32(id >> IndexBits); }
	};

#define GENERATE_FLAGS(E) \
	inline E operator|(E a, E b){ return E(u32(a) | u32(b)); } \
    inline E operator&(E a, E b){ return E(u32(a) & u32(b)); } \
    inline E& operator|=(E& a, E b){ a = a | b; return a; } \
    inline bool any(E v){ return u32(v) != 0; } \
    inline bool has(E v, E f){ return u32(v & f) != 0; }
}