#pragma once

#include <Runtime/Graphics/RHI/Shader/GfxShader.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	class GfxVkShader final : public GfxShader
	{
	public:
		GfxVkShader(const GfxShaderDesc& desc, GfxDevice* pDevice);
		~GfxVkShader() override final;

		void* Shader() const override final;

	private:
		VkShaderModule m_shader;
	};
}
