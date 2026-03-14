#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueueDesc.h>
#include <Runtime/Graphics/RHI/Util/PipelineStageFlags.h>

#include <vector>

namespace Horizon
{
	class GfxCommandBuffer;
	class GfxFence;
	class GfxSemaphore;

	class GfxQueue : public GfxObject
	{
	public:
		GfxQueue(const GfxQueueDesc& desc, GfxDevice* pDevice);
		virtual ~GfxQueue() override = default;

		virtual void* Queue() const = 0;
		virtual void Submit(const std::vector<GfxCommandBuffer*>& cmdBuffers, const std::vector<GfxSemaphore*>& waits, 
			const std::vector<GfxSemaphore*>& signals, const GfxFence* fence, PipelineStageFlags flags) const = 0;
		virtual void WaitIdle() const = 0;

		QueueType Type() const { return m_desc.type; }
		u32 FamilyIndex() const { return m_desc.familyIndex; }

	private:
		GfxQueueDesc m_desc;
	};
}
