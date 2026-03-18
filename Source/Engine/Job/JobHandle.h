#pragma once

#include <atomic>
#include <memory>

namespace Horizon
{
    struct JobState
    {
        std::atomic<bool> completed { false };
    };

    struct JobHandle
    {
        std::shared_ptr<JobState> m_state;
        bool IsComplete() const { return !m_state || m_state->completed.load(std::memory_order_acquire); }
    };
}
