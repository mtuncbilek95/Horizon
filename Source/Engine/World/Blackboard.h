#pragma once

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Sync/Mutex.h>
#include <Runtime/RTTR/TypeHandle.h>

#include <unordered_map>
#include <utility>

namespace Horizon::Engine
{
	class Blackboard final
	{
		struct Entry
		{
			virtual ~Entry() = default;
			virtual void* GetData() = 0;
		};

		template<typename T>
		struct TypedEntry final : Entry
		{
			template<typename... Args>
			explicit TypedEntry(Args&&... args) : m_value(std::forward<Args>(args)...) {}

			void* GetData() final { return &m_value; }

			T m_value;
		};

	public:
		Blackboard() = default;
		~Blackboard();

		Blackboard(const Blackboard&) = delete;
		Blackboard& operator=(const Blackboard&) = delete;

		template<typename T, typename... Args>
		T& Add(Args&&... args)
		{
			Reflect::TypeHandle handl = Reflect::TypeOf<T>();

			void* pExisting = Replace(handl);
			if (pExisting)
				return *static_cast<T*>(pExisting);

			UniquePtr<TypedEntry<T>> entry = MakeUnique<TypedEntry<T>>(std::forward<Args>(args)...);
			T* pValue = &entry->m_value;

			Insert(handl, std::move(entry));
			return *pValue;
		}

		template<typename T, typename... Args>
		T& GetOrAdd(Args&&... args)
		{
			Reflect::TypeHandle handl = Reflect::TypeOf<T>();

			void* pExisting = Find(handl, false);
			if (pExisting)
				return *static_cast<T*>(pExisting);

			UniquePtr<TypedEntry<T>> entry = MakeUnique<TypedEntry<T>>(std::forward<Args>(args)...);
			T* pValue = &entry->m_value;

			Insert(handl, std::move(entry));
			return *pValue;
		}

		template<typename T>
		const T* Get() const
		{
			return static_cast<const T*>(Find(Reflect::TypeOf<T>(), true));
		}

		template<typename T>
		const T* TryGet() const
		{
			return static_cast<const T*>(Find(Reflect::TypeOf<T>(), false));
		}

		template<typename T>
		b8 Has() const
		{
			return Find(Reflect::TypeOf<T>(), false) != nullptr;
		}

		void Reset();

	private:
		void* Replace(Reflect::TypeHandle handle);
		void Insert(Reflect::TypeHandle handle, UniquePtr<Entry> entry);
		void* Find(Reflect::TypeHandle handle, b8 logMissing) const;

	private:
		List<UniquePtr<Entry>> m_entries;
		std::unordered_map<Reflect::TypeHandle, usize> m_lookup;

		mutable PAL::Mutex m_lock;
	};
}