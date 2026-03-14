#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	class GfxVkFence : public GfxFence
	{
	public:
		GfxVkFence(const GfxFenceDesc& desc, GfxDevice* pDevice);
		~GfxVkFence() override final;

		void* Fence() const override final;
		void WaitIdle() const override final;
		void Reset() const override final;

	private:
		VkFence m_fence;
	};
}
