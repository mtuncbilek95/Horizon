#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace Horizon::Reflect
{
	class Base;
}

namespace Horizon
{
	namespace Detail
	{
		class SharedControlBlock
		{
		public:
			virtual ~SharedControlBlock() = default;

			void AddRef()
			{
				m_refCount.FetchAdd(1, PAL::MemoryOrder::Relaxed);
			}

			void ReleaseRef()
			{
				if (m_refCount.Subtract(1, PAL::MemoryOrder::AcqRel) == 0)
					delete this;
			}

			i64 GetRefCount() const { return m_refCount.Load(PAL::MemoryOrder::Relaxed); }

		private:
			PAL::Atomic<i64> m_refCount = 1;
		};

		template<typename T>
		class SharedPointerBlock final : public SharedControlBlock
		{
		public:
			explicit SharedPointerBlock(T* pPtr) : m_ptr(pPtr) {}

			~SharedPointerBlock() override
			{
				delete m_ptr;
			}

		private:
			T* m_ptr = nullptr;
		};

		template<typename T>
		class SharedInlineBlock final : public SharedControlBlock
		{
		public:
			template<typename... TArgs>
			explicit SharedInlineBlock(TArgs&&... args) : m_value(std::forward<TArgs>(args)...) {}

			T* Get() { return &m_value; }

		private:
			T m_value;
		};
	}

	template<typename T>
		requires (!std::is_base_of_v<Reflect::Base, std::remove_cv_t<T>>)
	class SharedPtr final
	{
		template<typename U>
			requires (!std::is_base_of_v<Reflect::Base, std::remove_cv_t<U>>)
		friend class SharedPtr;

		template<typename U, typename... TArgs>
		friend SharedPtr<U> MakeShared(TArgs&&... args);

	public:
		SharedPtr() = default;
		SharedPtr(std::nullptr_t) {}

		explicit SharedPtr(T* pPtr) : m_ptr(pPtr)
		{
			if (pPtr)
				m_block = new Detail::SharedPointerBlock<T>(pPtr);
		}

		SharedPtr(const SharedPtr& other) : m_ptr(other.m_ptr), m_block(other.m_block)
		{
			if (m_block)
				m_block->AddRef();
		}

		template<typename U>
			requires std::convertible_to<U*, T*>
		SharedPtr(const SharedPtr<U>& other) : m_ptr(other.m_ptr), m_block(other.m_block)
		{
			if (m_block)
				m_block->AddRef();
		}

		SharedPtr(SharedPtr&& other) noexcept : m_ptr(std::exchange(other.m_ptr, nullptr)), m_block(std::exchange(other.m_block, nullptr)) {}

		template<typename U>
			requires std::convertible_to<U*, T*>
		SharedPtr(SharedPtr<U>&& other) noexcept : m_ptr(std::exchange(other.m_ptr, nullptr)), m_block(std::exchange(other.m_block, nullptr)) {}

		~SharedPtr()
		{
			if (m_block)
				m_block->ReleaseRef();
		}

		SharedPtr& operator=(const SharedPtr& other)
		{
			SharedPtr(other).Swap(*this);
			return *this;
		}

		template<typename U>
			requires std::convertible_to<U*, T*>
		SharedPtr& operator=(const SharedPtr<U>& other)
		{
			SharedPtr(other).Swap(*this);
			return *this;
		}

		SharedPtr& operator=(SharedPtr&& other) noexcept
		{
			SharedPtr(std::move(other)).Swap(*this);
			return *this;
		}

		template<typename U>
			requires std::convertible_to<U*, T*>
		SharedPtr& operator=(SharedPtr<U>&& other) noexcept
		{
			SharedPtr(std::move(other)).Swap(*this);
			return *this;
		}

		SharedPtr& operator=(std::nullptr_t) noexcept
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
			SharedPtr().Swap(*this);
		}

		void Reset(T* pPtr)
		{
			SharedPtr(pPtr).Swap(*this);
		}

		void Swap(SharedPtr& other) noexcept
		{
			std::swap(m_ptr, other.m_ptr);
			std::swap(m_block, other.m_block);
		}

		b8 operator==(std::nullptr_t) const { return m_ptr == nullptr; }

		template<typename U>
		b8 operator==(const SharedPtr<U>& other) const { return m_ptr == other.m_ptr; }

	private:
		SharedPtr(T* pPtr, Detail::SharedControlBlock* pBlock) : m_ptr(pPtr), m_block(pBlock) {}

	private:
		T* m_ptr = nullptr;
		Detail::SharedControlBlock* m_block = nullptr;
	};

	template<typename T, typename... TArgs>
	SharedPtr<T> MakeShared(TArgs&&... args)
	{
		auto* pBlock = new Detail::SharedInlineBlock<T>(std::forward<TArgs>(args)...);
		return SharedPtr<T>(pBlock->Get(), pBlock);
	}
}