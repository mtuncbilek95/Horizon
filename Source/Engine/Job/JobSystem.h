#pragma once

#include <Engine/Job/IJob.h>
#include <Engine/Job/JobHandle.h>
#include <Engine/System/System.h>

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace Horizon
{
    class JobSystem final : public System<JobSystem>
    {
    public:
        JobSystem();
        ~JobSystem() = default;

        template<typename Func>
        JobHandle Submit(Func&& func, std::vector<JobHandle> deps = {})
        {
            struct JobImpl : IJob
            {
                Func f;
                JobImpl(Func&& f) : f(std::forward<Func>(f)) {}
                void Execute() override { f(); }
            };

            JobHandle handle{ std::make_shared<JobState>() };
            SubmitInternal(std::make_unique<JobImpl>(std::forward<Func>(func)), handle, std::move(deps));
            return handle;
        }

    private:
        EngineReport OnInitialize() final;
        void OnSync() final;
        void OnFinalize() final;

        void SubmitInternal(std::unique_ptr<IJob> job, JobHandle handle, std::vector<JobHandle> deps);
        void WorkerLoop();
        void PromotePending();

        struct JobEntry
        {
            std::unique_ptr<IJob> job;
            JobHandle handle;
            std::vector<JobHandle> dependencies;

            bool IsReady() const
            {
                for (const auto& dep : dependencies)
                    if (!dep.IsComplete()) return false;
                return true;
            }
        };

        std::vector<std::thread> m_workers;
        std::deque<JobEntry> m_readyQueue;
        std::vector<JobEntry> m_pendingQueue;

        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::atomic<bool> m_running{ false };
    };
}
