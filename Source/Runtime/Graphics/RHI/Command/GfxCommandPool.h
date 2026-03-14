#pragma once

#include <Runtime/Graphics/RHI/Command/GfxCommandPoolDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

#include <memory>

namespace Horizon
{
	class GfxCommandBuffer;

	/**
	 * @class GfxCommandPool
	 * @brief All purpose command pool for RHI. CommandPool a.k.a
	 * CommandAllocator is being used for pooling the commandbuffers.
	 */
	class GfxCommandPool : public GfxObject
	{
	public:
		GfxCommandPool(const GfxCommandPoolDesc& desc, GfxDevice* pDevice);
		virtual ~GfxCommandPool() override = default;

		virtual void* Pool() const = 0;
		virtual std::shared_ptr<GfxCommandBuffer> CreateBuffer(CommandLevel lvl) = 0;

	private:
		GfxCommandPoolDesc m_desc;
	};
}
