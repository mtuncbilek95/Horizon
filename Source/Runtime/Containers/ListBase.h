#pragma once

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <cstring>
#include <new>
#include <utility>

namespace Horizon
{
	class H_EXPORT ListBase
	{
	public:
		struct ElementOps
		{
			void (*constructAt)(void*);
			void (*destructAt)(void*);
			void (*relocate)(void* pDst, void* pSrc);
		};

	public:
		usize GetCount() const { return m_count; }
		usize GetCapacity() const { return m_capacity; }

		void* GetData() { return m_data; }
		const void* GetData() const { return m_data; }

		void* GetElement(usize index, usize elemSize) { return static_cast<c8*>(m_data) + index * elemSize; }
		const void* GetElement(usize index, usize elemSize) const { return static_cast<const c8*>(m_data) + index * elemSize; }

		void ResizeErased(usize count, usize elemSize, const ElementOps* pOps)
		{
			if (count > m_capacity)
				ReallocateErased(count, elemSize, pOps);

			if (count > m_count)
			{
				if (pOps)
				{
					for (usize i = m_count; i < count; ++i)
						pOps->constructAt(GetElement(i, elemSize));
				}
				else
				{
					std::memset(GetElement(m_count, elemSize), 0, (count - m_count) * elemSize);
				}
			}
			else if (count < m_count)
			{
				if (pOps)
				{
					for (usize i = count; i < m_count; ++i)
						pOps->destructAt(GetElement(i, elemSize));
				}
			}

			m_count = count;
		}

	protected:
		void ReallocateErased(usize capacity, usize elemSize, const ElementOps* pOps)
		{
			void* pNewBuffer = ::operator new(elemSize * capacity);

			if (pOps)
			{
				for (usize i = 0; i < m_count; ++i)
					pOps->relocate(static_cast<c8*>(pNewBuffer) + i * elemSize, GetElement(i, elemSize));
			}
			else if (m_count > 0)
			{
				std::memcpy(pNewBuffer, m_data, m_count * elemSize);
			}

			if (m_data)
				::operator delete(m_data);

			m_data = pNewBuffer;
			m_capacity = capacity;
		}

	protected:
		void* m_data = nullptr;
		usize m_count = 0;
		usize m_capacity = 0;
	};

	template<typename E>
	const ListBase::ElementOps* ElementOpsFor()
	{
		static const ListBase::ElementOps ops = {
			[](void* pMem) { ::new (pMem) E(); },
			[](void* pMem) { static_cast<E*>(pMem)->~E(); },
			[](void* pDst, void* pSrc)
			{
				::new (pDst) E(std::move(*static_cast<E*>(pSrc)));
				static_cast<E*>(pSrc)->~E();
			}
		};

		return &ops;
	}
}
