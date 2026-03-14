#pragma once

#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Horizon
{
	class GfxVkQueue final : public GfxQueue
	{
		friend class GfxVkDevice;

	public:
		GfxVkQueue(const GfxQueueDesc& desc, GfxDevice* pDevice);
		~GfxVkQueue() override;

		void* Queue() const override final;
		void Submit(const std::vector<GfxCommandBuffer*>& cmdBuffers, const std::vector<GfxSemaphore*>& waits,
			const std::vector<GfxSemaphore*>& signals, const GfxFence* fence, PipelineStageFlags flags) const override final;
		void WaitIdle() const override final;

	private:
		VkQueue m_queue;
	};
}
