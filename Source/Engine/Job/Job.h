#pragma once

#include <Runtime/Definitions/Allocator.h>

#include <concepts>
#include <type_traits>
#include <utility>

namespace Horizon::Engine
{
	using JobFunction = void(*)(void* userData);

	class H_EXPORT Job
	{
		JobFunction execute = nullptr;
		JobFunction discard = nullptr;
		void* userData = nullptr;

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

		b8 IsValid() const { return execute != nullptr; }

		void Execute();
		void Discard();

		template<typename TCallable>
		static Job Bind(TCallable&& callable)
		{
			using Stored = std::decay_t<TCallable>;

			Stored* stored = Memory::Allocator::Create<Stored>(Memory::CurrLoc(), std::forward<TCallable>(callable));

			Job job;
			job.userData = stored;
			job.execute = [](void* pData)
				{
					Stored* pTarget = (Stored*)pData;
					(*pTarget)();
					Memory::Allocator::Delete(pTarget);
				};

			job.discard = [](void* pData)
				{
					Memory::Allocator::Delete((Stored*)pData);
				};

			return job;
		}

	private:
		void Release();
	};
}