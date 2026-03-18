#include "JobSystem.h"

namespace Horizon
{
    JobSystem::JobSystem()
    {
    }

    EngineReport JobSystem::OnInitialize()
    {
        m_running = true;

        u32 threadCount = std::max(1u, std::thread::hardware_concurrency() - 1);
        m_workers.reserve(threadCount);

        for (u32 i = 0; i < threadCount; i++)
            m_workers.emplace_back(&JobSystem::WorkerLoop, this);

        Log::Terminal(LogType::Success, GetObjectType(), "Started {} worker threads", threadCount);
        return EngineReport();
    }

    void JobSystem::OnSync()
    {
    }

    void JobSystem::OnFinalize()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = false;
        }

        m_cv.notify_all();

        for (auto& worker : m_workers)
            worker.join();

        m_workers.clear();
    }

    void JobSystem::SubmitInternal(std::unique_ptr<IJob> job, JobHandle handle, std::vector<JobHandle> deps)
    {
        JobEntry entry { std::move(job), handle, std::move(deps) };

        std::lock_guard<std::mutex> lock(m_mutex);

        if (entry.IsReady())
        {
            m_readyQueue.push_back(std::move(entry));
            m_cv.notify_one();
        }
        else
        {
            m_pendingQueue.push_back(std::move(entry));
        }
    }

    void JobSystem::WorkerLoop()
    {
        while (true)
        {
            JobEntry entry;

            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [this] { return !m_readyQueue.empty() || !m_running; });

                if (!m_running && m_readyQueue.empty())
                    return;

                entry = std::move(m_readyQueue.front());
                m_readyQueue.pop_front();
            }

            entry.job->Execute();
            entry.handle.m_state->completed.store(true, std::memory_order_release);

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                PromotePending();
            }
        }
    }

    void JobSystem::PromotePending()
    {
        for (auto it = m_pendingQueue.begin(); it != m_pendingQueue.end();)
        {
            if (it->IsReady())
            {
                m_readyQueue.push_back(std::move(*it));
                it = m_pendingQueue.erase(it);
                m_cv.notify_one();
            }
            else
            {
                ++it;
            }
        }
    }
}
