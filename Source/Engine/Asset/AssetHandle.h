#pragma once

#include <Runtime/Containers/Guid.h>

#include <utility>

namespace Horizon
{
	class AssetModule;

	template<typename T>
	class AssetHandle
	{
		friend class AssetModule;
	public:
		AssetHandle() = default;
		AssetHandle(const AssetHandle& other) : m_owner(other.m_owner),
			m_slot(other.m_slot), m_generation(other.m_generation)
		{
			if (m_owner)
				m_owner->IncrementRef(m_slot);
		}

		AssetHandle(AssetHandle&& o) noexcept : m_owner(o.m_owner), m_slot(o.m_slot), 
			m_generation(o.m_generation) 
		{
			o.m_owner = nullptr;
			o.m_slot = kInvalid32;
		}

		AssetHandle& operator=(AssetHandle other) noexcept 
		{
			swap(*this, other);
			return *this;
		}

		~AssetHandle()
		{
			if (m_owner)
				m_owner->DecrementRef(m_slot);
		}

		friend void swap(AssetHandle& a, AssetHandle& b) noexcept 
		{
			using std::swap;
			swap(a.m_owner, b.m_owner);
			swap(a.m_slot, b.m_slot);
			swap(a.m_generation, b.m_generation);
		}

		b8 IsValid() const { return m_owner != nullptr && m_slot != kInvalid32; }
		explicit operator b8() const { return IsValid(); }

		b8 IsReady() const { return m_owner && m_owner->IsSlotReady(m_slot, m_generation); }
		const T* Get() const
		{
			const void* pPtr = m_owner ? m_owner->ResolveReady(m_slot, m_generation) : nullptr;
			return static_cast<const T*>(pPtr);
		}

		const T* operator->() const { return Get(); }
		const T& operator*() const { return *Get(); }

		Guid GetGuid() const { return m_owner ? m_owner->GuidOf(m_slot, m_generation) : Guid{}; }

	private:
		AssetHandle(AssetModule* pOwner, u32 slot, u32 gen) : m_owner(pOwner),
			m_slot(slot), m_generation(gen)
		{
			if (m_owner)
				m_owner->IncrementRef(m_slot);
		}

	private:
		AssetModule* m_owner = nullptr;
		u32 m_slot = kInvalid32;
		u32 m_generation = 0;
	};
}