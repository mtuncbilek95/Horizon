#pragma once

#include <Engine/Core/System.h>

#include <vector>
#include <thread>
#include <functional>
#include <queue>
#include <condition_variable>
#include <memory>

namespace Horizon
{
	using Job = std::function<void()>;

	class JobSystem : public System<JobSystem>
	{
	public:
		void Submit(std::function<void()> job);
		u32 WorkerCount() const;

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

		void WorkerLoop(u32 workerIndex);

		std::queue<Job> m_jobQueue;
		std::mutex m_mutex;
		std::condition_variable m_condition;
		std::atomic<b8> m_shutdown = false;
		std::vector<std::thread> m_workers;
	};
}