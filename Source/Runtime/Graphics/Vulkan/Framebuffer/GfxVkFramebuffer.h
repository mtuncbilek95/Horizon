#pragma once

#include <Runtime/Graphics/RHI/Framebuffer/GfxFramebuffer.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Horizon
{
	class GfxVkFramebuffer final : public GfxFramebuffer
	{
	public:
		GfxVkFramebuffer(const GfxFramebufferDesc& desc, GfxDevice* pDevice);
		~GfxVkFramebuffer() override final;

		void* Framebuffer(usize index = 0) const override final;
		void Resize(const Math::Vec3u& size) override final;

	private:
		std::vector<VkFramebuffer> m_framebuffers;
	};
}
