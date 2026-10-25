#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

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
	template<typename T>
		requires (!std::is_base_of_v<Reflect::Base, std::remove_cv_t<T>>)
	class UniquePtr final
	{
		template<typename U>
			requires (!std::is_base_of_v<Reflect::Base, std::remove_cv_t<U>>)
		friend class UniquePtr;

	public:
		UniquePtr() = default;
		UniquePtr(std::nullptr_t) {}
		explicit UniquePtr(T* pPtr) : m_ptr(pPtr) {}

		UniquePtr(const UniquePtr&) = delete;
		UniquePtr& operator=(const UniquePtr&) = delete;

		UniquePtr(UniquePtr&& other) noexcept : m_ptr(other.Release()) {}

		template<typename U>
			requires std::convertible_to<U*, T*>
		UniquePtr(UniquePtr<U>&& other) noexcept : m_ptr(other.Release()) {}

		~UniquePtr()
		{
			Destroy(m_ptr);
		}

		UniquePtr& operator=(UniquePtr&& other) noexcept
		{
			if (this != &other)
				Reset(other.Release());

			return *this;
		}

		template<typename U>
			requires std::convertible_to<U*, T*>
		UniquePtr& operator=(UniquePtr<U>&& other) noexcept
		{
			Reset(other.Release());
			return *this;
		}

		UniquePtr& operator=(std::nullptr_t) noexcept
		{
			Reset();
			return *this;
		}

		T* Get() const { return m_ptr; }
		T& operator*() const { return *m_ptr; }
		T* operator->() const { return m_ptr; }
		explicit operator b8() const { return m_ptr != nullptr; }

		T* Release()
		{
			return std::exchange(m_ptr, nullptr);
		}

		void Reset(T* pPtr = nullptr)
		{
			Destroy(std::exchange(m_ptr, pPtr));
		}

		void Swap(UniquePtr& other) noexcept
		{
			std::swap(m_ptr, other.m_ptr);
		}

		b8 operator==(std::nullptr_t) const { return m_ptr == nullptr; }

		template<typename U>
		b8 operator==(const UniquePtr<U>& other) const { return m_ptr == other.m_ptr; }

	private:
		static void Destroy(T* pPtr) { delete pPtr; }

	private:
		T* m_ptr = nullptr;
	};

	template<typename T, typename... TArgs>
		requires (!std::is_base_of_v<Reflect::Base, std::remove_cv_t<T>>)
	UniquePtr<T> MakeUnique(TArgs&&... args)
	{
		return UniquePtr<T>(new T(std::forward<TArgs>(args)...));
	}
}