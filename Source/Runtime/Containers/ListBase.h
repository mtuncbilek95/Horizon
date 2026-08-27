#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon
{
	class H_EXPORT ListBase
	{
	public:
		ListBase() = default;
		virtual ~ListBase() = default;

		ListBase(const ListBase&) = default;
		ListBase& operator=(const ListBase&) = default;

		ListBase(ListBase&&) noexcept = default;
		ListBase& operator=(ListBase&&) noexcept = default;

		virtual usize GetElementSize() const = 0;
		virtual usize GetCount() const = 0;

		virtual void Resize(usize count) = 0;
		virtual void RemoveAt(usize index) = 0;

		virtual void* GetElementAt(usize index) = 0;
		virtual const void* GetElementAt(usize index) const = 0;
	};
}