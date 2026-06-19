#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon
{
	using HandleId = u64;
	inline constexpr HandleId InvalidHandleId = ~0ull;
	inline constexpr HandleId IndexBits = 32;
	inline constexpr HandleId IndexMask = (HandleId(1) << IndexBits) - 1;

	template<typename Tag>
	struct Handle
	{
		u32 index = kInvalid32;

		b8 IsValid() const { return index != kInvalid32; }

		explicit operator b8() const { return IsValid(); }

		b8 operator==(const Handle& other) const { return index == other.index; }
		b8 operator!=(const Handle& other) const { return index != other.index; }

		u32 Index() const { return index; }
	};

	template<typename Tag>
	struct HandleEx
	{
		HandleId id = InvalidHandleId;

		b8 IsValid() const { return id != InvalidHandleId; }

		explicit operator b8() const { return IsValid(); }

		b8 operator==(const HandleEx& other) const { return id == other.id; }
		b8 operator!=(const HandleEx& other) const { return id != other.id; }

		static HandleEx Generate(u32 index, u32 generation)
		{
			HandleEx hndl;
			hndl.id = (HandleId(generation) << IndexBits) | HandleId(index);
			return hndl;
		}

		u32 Index() const { return u32(id & IndexMask); }
		u32 Generation() const { return u32(id >> IndexBits); }
	};
}
