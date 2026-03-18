#pragma once

#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

#include <vector>

namespace Horizon
{
    class GfxVkQueue final : public GfxQueue
    {
        friend class GfxVkDevice;

    public:
        GfxVkQueue(const GfxQueueDesc& desc, GfxDevice* pDevice);
        ~GfxVkQueue() override;

        void* Queue() const final;
        void Submit(const std::vector<GfxCommandBuffer*>& cmdBuffers, const std::vector<GfxSemaphore*>& waits,
            const std::vector<GfxSemaphore*>& signals, const GfxFence* fence, PipelineStageFlags flags) const final;
        void WaitIdle() const final;

    private:
        VkQueue m_queue;
    };
}
