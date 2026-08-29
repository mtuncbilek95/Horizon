#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <source_location>
#include <type_traits>
#include <utility>
#include <new>

namespace Horizon::Memory
{
	using SourceLocation = std::source_location;
	inline constexpr SourceLocation CurrLoc(SourceLocation loc = SourceLocation::current()) noexcept { return loc; }

	struct Allocator
	{
		template<typename T, typename... Args>
		static T* Create(SourceLocation loc, Args&&... args)
		{
			void* mem = AllocateRaw(sizeof(T), alignof(T), loc);

			if (!mem)
				return nullptr;

			return ::new (mem) T(std::forward<Args>(args)...);
		}

		template<typename T>
		static void Delete(T* pAddress)
		{
			if (!pAddress)
				return;

			void* pBase = pAddress;

			if constexpr (std::is_polymorphic_v<T>)
				pBase = dynamic_cast<void*>(pAddress);

			pAddress->~T();
			FreeRaw(pBase);
		}

		static void* AllocateRaw(usize size, usize align, SourceLocation loc);
		static void* ReallocateRaw(void* pAddress, usize newSize, usize align, SourceLocation loc);
		static void FreeRaw(void* pAddress);

		static void ReportLeaks();
		static b8 IsTrackingEnabled();
	};
}