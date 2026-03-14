#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	class GfxVkSemaphore final : public GfxSemaphore
	{
	public:
		GfxVkSemaphore(const GfxSemaphoreDesc& desc, GfxDevice* pDevice);
		~GfxVkSemaphore() override final;

		void* Semaphore() const override final;
		void WaitIdle() const override final;
		void Reset() const override final;
		
	private:
		VkSemaphore m_semaphore;
	};
}
