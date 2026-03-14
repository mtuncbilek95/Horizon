#pragma once

#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	class GfxVkPipeline : public GfxPipeline
	{
	public:
		GfxVkPipeline(const GfxGraphicsPipelineDesc& desc, GfxDevice* pDevice);
		GfxVkPipeline(const GfxComputePipelineDesc& desc, GfxDevice* pDevice);
		~GfxVkPipeline() override final;

		void* Pipeline() const override final;
		void* PipelineLayout() const override final;
		void* PipelineCache() const override final;

	private:
		VkPipeline m_pipeline;
		VkPipelineLayout m_layout;
		VkPipelineCache m_cache;
	};
}
