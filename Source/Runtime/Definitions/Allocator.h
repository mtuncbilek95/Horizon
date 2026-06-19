#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <source_location>
#include <utility>
#include <new>

namespace Horizon
{
	using SourceLocation = std::source_location;
	inline constexpr SourceLocation CurrLoc(SourceLocation loc = SourceLocation::current()) noexcept { return loc; }

	struct Allocator
	{
		template<typename T, typename... Args>
		static T* Create(SourceLocation loc, Args&&... args)
		{
			void* mem = AllocateRaw(sizeof(T), alignof(T), loc);
			return ::new (mem) T(std::forward<Args>(args)...);
		}

		template<typename T>
		static void Delete(T* pAddress)
		{
			if (!pAddress) 
				return;

			pAddress->~T();
			FreeRaw(pAddress);
		}

		static void ReportLeaks();

	private:
		static void* AllocateRaw(usize size, usize align, SourceLocation loc);
		static void FreeRaw(void* p);
	};
}