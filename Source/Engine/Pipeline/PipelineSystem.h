#pragma once

#include <Engine/System/System.h>
#include <Engine/Pipeline/GraphicsPipelineStateDesc.h>

#include <unordered_map>
#include <memory>

namespace Horizon
{
	class GfxPipeline;

	class PipelineSystem : public System<PipelineSystem>
	{
	public:
		PipelineSystem() = default;
		virtual ~PipelineSystem() = default;
		
		SystemReport OnInitialize() override;
		void OnSync() override;
		void OnFinalize() override;

		GfxPipeline* GetGraphicsPipeline(const GraphicsPipelineStateDesc& desc);

	private:
		GfxPipeline* CreateGraphicsPipeline(const GraphicsPipelineStateDesc& desc);
		u64 GetGraphicsPipelineHash(const GraphicsPipelineStateDesc& desc) const;

	private:
		std::unordered_map<u64, std::shared_ptr<GfxPipeline>> m_pipelines;
	};
}