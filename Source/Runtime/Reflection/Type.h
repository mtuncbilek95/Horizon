#pragma once

namespace Horizon
{
	struct ReflectionTypeTag {};
	using ReflectionTypeHandle = Handle<ReflectionTypeTag>;

	namespace Detail
	{
		constexpr u64 Fnv1a(const char* signature)
		{
			u64 hash = 14695981039346656037ull;
			while (*signature)
			{
				hash ^= (u64)(u8)(*signature++);
				hash *= 1099511628211ull;
			}
			return hash;
		}
	}

	template<typename T>
	constexpr ReflectionTypeHandle TypeIdOf()
	{
		return ReflectionTypeHandle::Generate(Detail::Fnv1a(__FUNCSIG__));
	}
}