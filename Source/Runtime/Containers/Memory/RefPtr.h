#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RTTR/Base.h>
#include <Runtime/RTTR/Type.h>

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace Horizon
{
	namespace Detail
	{
		class RefControlBlock final
		{
		public:
			explicit RefControlBlock(Reflect::Base* pObject) : m_object(pObject) {}

			void AddRef()
			{
				m_refCount.FetchAdd(1, PAL::MemoryOrder::Relaxed);
			}

			void ReleaseRef()
			{
				if (m_refCount.Subtract(1, PAL::MemoryOrder::AcqRel) != 0)
					return;

				delete m_object;
				delete this;
			}

			i64 GetRefCount() const { return m_refCount.Load(PAL::MemoryOrder::Relaxed); }

		private:
			Reflect::Base* m_object = nullptr;
			PAL::Atomic<i64> m_refCount = 1;
		};
	}

	template<typename T>
		requires (std::is_base_of_v<Reflect::Base, std::remove_cv_t<T>>)
	class RefPtr final
	{
		template<typename U>
			requires (std::is_base_of_v<Reflect::Base, std::remove_cv_t<U>>)
		friend class RefPtr;

		template<typename U>
		friend RefPtr<U> MakeRef(const Reflect::Type* pType);

	public:
		RefPtr() = default;
		RefPtr(std::nullptr_t) {}

		RefPtr(const RefPtr& other) : m_ptr(other.m_ptr), m_block(other.m_block)
		{
			if (m_block)
				m_block->AddRef();
		}

		template<typename U>
			requires std::convertible_to<U*, T*>
		RefPtr(const RefPtr<U>& other) : m_ptr(other.m_ptr), m_block(other.m_block)
		{
			if (m_block)
				m_block->AddRef();
		}

		RefPtr(RefPtr&& other) noexcept : m_ptr(std::exchange(other.m_ptr, nullptr)), m_block(std::exchange(other.m_block, nullptr)) {}

		template<typename U>
			requires std::convertible_to<U*, T*>
		RefPtr(RefPtr<U>&& other) noexcept : m_ptr(std::exchange(other.m_ptr, nullptr)), m_block(std::exchange(other.m_block, nullptr)) {}

		~RefPtr()
		{
			if (m_block)
				m_block->ReleaseRef();
		}

		RefPtr& operator=(const RefPtr& other)
		{
			RefPtr(other).Swap(*this);
			return *this;
		}

		template<typename U>
			requires std::convertible_to<U*, T*>
		RefPtr& operator=(const RefPtr<U>& other)
		{
			RefPtr(other).Swap(*this);
			return *this;
		}

		RefPtr& operator=(RefPtr&& other) noexcept
		{
			RefPtr(std::move(other)).Swap(*this);
			return *this;
		}

		template<typename U>
			requires std::convertible_to<U*, T*>
		RefPtr& operator=(RefPtr<U>&& other) noexcept
		{
			RefPtr(std::move(other)).Swap(*this);
			return *this;
		}

		RefPtr& operator=(std::nullptr_t) noexcept
		{
			Reset();
			return *this;
		}

		T* Get() const { return m_ptr; }
		T& operator*() const { return *m_ptr; }
		T* operator->() const { return m_ptr; }
		explicit operator b8() const { return m_ptr != nullptr; }

		i64 GetRefCount() const { return m_block ? m_block->GetRefCount() : 0; }

		void Reset()
		{
			RefPtr().Swap(*this);
		}

		void Swap(RefPtr& other) noexcept
		{
			std::swap(m_ptr, other.m_ptr);
			std::swap(m_block, other.m_block);
		}

		b8 operator==(std::nullptr_t) const { return m_ptr == nullptr; }

		template<typename U>
		b8 operator==(const RefPtr<U>& other) const { return m_ptr == other.m_ptr; }

	private:
		RefPtr(T* pPtr, Detail::RefControlBlock* pBlock) : m_ptr(pPtr), m_block(pBlock) {}

	private:
		T* m_ptr = nullptr;
		Detail::RefControlBlock* m_block = nullptr;
	};

	template<typename T>
	RefPtr<T> MakeRef(const Reflect::Type* pType)
	{
		if (!pType)
		{
			Terminal::Error("RefPtr", "Cannot make reference from a null type");
			return {};
		}

		auto* pObject = static_cast<T*>(pType->Create());

		if (!pObject)
			return {};

		return RefPtr<T>(pObject, new Detail::RefControlBlock(pObject));
	}
}