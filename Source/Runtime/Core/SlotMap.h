#pragma once

#include <array>
#include <shared_mutex>

namespace Horizon
{
	struct NullMutex
	{
		void lock() {}
		void unlock() {}
		bool try_lock() { return true; }
		void lock_shared() {}
		void unlock_shared() {}
		bool try_lock_shared() { return true; }
	};

	template<typename Object, typename Handle, usize Capacity = 4096, b8 ThreadSafe = true>
	class ObjectSlotMap final
	{
		using MutexType = std::conditional_t<ThreadSafe, std::shared_mutex, NullMutex>;

	public:
		ObjectSlotMap()
		{
			for (usize i = 0; i < Capacity; i++)
				m_freeList[i] = static_cast<u32>(Capacity - 1 - i);

			m_freeTop = Capacity;
		}

		~ObjectSlotMap()
		{
			for (usize i = 0; i < Capacity; i++)
			{
				if (m_alive[i])
					m_data[i].~Object();
			}
		}

		ObjectSlotMap(const ObjectSlotMap&) = delete;
		ObjectSlotMap& operator=(const ObjectSlotMap&) = delete;
		ObjectSlotMap(ObjectSlotMap&&) = delete;
		ObjectSlotMap& operator=(ObjectSlotMap&&) = delete;

		[[nodiscard]] Handle Insert(Object&& obj)
		{
			std::unique_lock lock(m_mutex);

			assert(m_freeTop > 0 && "ObjectSlotMap has reached its capacity");

			u32 slot = m_freeList[--m_freeTop];
			new (m_data + slot) Object(std::move(obj));
			m_alive[slot] = true;
			m_count++;

			return Handle::Generate(slot, m_generations[slot]);
		}

		void Remove(Handle handle)
		{
			std::unique_lock lock(m_mutex);

			assert(IsValidUnsafe(handle) && "Invalid handle to remove");

			u32 slot = handle.Index();
			m_data[slot].~Object();
			m_alive[slot] = false;
			m_generations[slot]++;
			m_freeList[m_freeTop++] = slot;
			m_count--;
		}

		void Clear()
		{
			std::unique_lock lock(m_mutex);

			for (usize i = 0; i < Capacity; i++)
			{
				if (m_alive[i])
				{
					m_data[i].~Object();
					m_alive[i] = false;
					m_generations[i]++;
				}
				m_freeList[i] = static_cast<u32>(Capacity - 1 - i);
			}

			m_freeTop = Capacity;
			m_count = 0;
		}

		[[nodiscard]] Object* GetObject(Handle handle)
		{
			std::shared_lock lock(m_mutex);

			if (!IsValidUnsafe(handle))
				return nullptr;

			return m_data + handle.Index();
		}

		template<typename Func>
		b8 ResolveWrite(Handle handle, Func&& fn)
		{
			std::unique_lock lock(m_mutex);

			if (!IsValidUnsafe(handle))
				return false;

			fn(m_data[handle.Index()]);
			return true;
		}

		template<typename Func> 
		b8 ResolveRead(Handle handle, Func&& fn) const
		{
			std::shared_lock lock(m_mutex);

			if (!IsValidUnsafe(handle))
				return false;

			fn(static_cast<const Object&>(m_data[handle.Index()]));
			return true;
		}

		[[nodiscard]] b8 IsValid(Handle handle) const
		{
			std::shared_lock lock(m_mutex);
			return IsValidUnsafe(handle);
		}

		[[nodiscard]] usize GetSize() const { std::shared_lock lock(m_mutex); return m_count; }
		[[nodiscard]] usize GetCapacity() const { return Capacity; }
		[[nodiscard]] b8 IsEmpty() const { std::shared_lock lock(m_mutex); return m_count == 0; }
		[[nodiscard]] b8 IsFull() const { std::shared_lock lock(m_mutex); return m_count == Capacity; }

	private:
		[[nodiscard]] b8 IsValidUnsafe(Handle handle) const 
		{
			if (!handle.IsValid())
				return false;

			u32 slot = handle.Index();

			if (slot >= Capacity)
				return false;

			if (!m_alive[slot])
				return false;

			return m_generations[slot] == handle.Generation();
		}

	private:
		alignas(Object) std::array<std::byte, Capacity * sizeof(Object)> m_storage;
		Object* m_data = reinterpret_cast<Object*>(m_storage.data());

		std::array<u32, Capacity> m_generations = {};
		std::array<u32, Capacity> m_freeList = {};
		std::array<b8, Capacity> m_alive = {};
		u32 m_freeTop = 0;
		u32 m_count = 0;

		mutable MutexType m_mutex;
	};
}