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
		u64 index = kInvalid64;

		b8 IsValid() const { return index != kInvalid64; }

		explicit operator b8() const { return IsValid(); }

		b8 operator==(const Handle& other) const { return index == other.index; }
		b8 operator!=(const Handle& other) const { return index != other.index; }

		u64 Index() const { return index; }

		static Handle Generate(u64 index)
		{
			Handle hndl;
			hndl.index = index;
			return hndl;
		}
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

namespace std
{
	template<typename Tag>
	struct hash<Horizon::Handle<Tag>>
	{
		size_t operator()(const Horizon::Handle<Tag>& h) const noexcept
		{
			return std::hash<uint64_t>{}(h.index);
		}
	};

	template<typename Tag>
	struct hash<Horizon::HandleEx<Tag>>
	{
		size_t operator()(const Horizon::HandleEx<Tag>& h) const noexcept
		{
			return std::hash<uint64_t>{}(h.index);
		}
	};
}