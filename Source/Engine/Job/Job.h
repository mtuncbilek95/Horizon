#pragma once

#include <Runtime/Definitions/Allocator.h>

#include <concepts>
#include <type_traits>
#include <utility>

namespace Horizon::Engine
{
	using JobFunction = void(*)(void* pUserData);

	class H_EXPORT Job
	{
		JobFunction m_execute = nullptr;
		JobFunction m_discard = nullptr;
		void* m_userData = nullptr;

	public:
		Job() = default;
		Job(JobFunction function, void* pData = nullptr);
		Job(JobFunction function, JobFunction release, void* pData = nullptr);

		template<typename T>
			requires std::invocable<T> && (!std::convertible_to<T, JobFunction>)
		Job(T&& callable) : Job(Bind(std::forward<T>(callable))) {}

		Job(const Job&) = delete;
		Job& operator=(const Job&) = delete;

		Job(Job&& other) noexcept;
		Job& operator=(Job&& other) noexcept;

		~Job();

		b8 IsValid() const { return m_execute != nullptr; }

		void Execute();
		void Discard();

		template<typename TCallable>
		static Job Bind(TCallable&& callable)
		{
			using Stored = std::decay_t<TCallable>;

			Stored* pStored = Memory::Allocator::Create<Stored>(Memory::CurrLoc(), std::forward<TCallable>(callable));

			Job job;
			job.m_userData = pStored;
			job.m_execute = [](void* pData)
				{
					Stored* pTarget = (Stored*)pData;
					(*pTarget)();
					Memory::Allocator::Delete(pTarget);
				};

			job.m_discard = [](void* pData)
				{
					Memory::Allocator::Delete((Stored*)pData);
				};

			return job;
		}

	private:
		void Release();
	};
}