#include "PipelineContext.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

#include <Engine/Graphics/GfxContext.h>
#include <Engine/Graphics/ShaderContext.h>

namespace Horizon
{
	GfxPipeline* PipelineContext::GetOrCreate(const PipelineInfo& desc)
	{
		auto it = m_pipelines.find(desc);
		if (it != m_pipelines.end())
		{
			return it->second.get();
		}

		Log::Terminal(LogType::Warning, "PipelineContext", "{} has not been found! Creating new...", desc.cacheName);

		auto& shaderCtx = RequestContext<ShaderContext>();
		auto& gDevice = RequestContext<GfxContext>().Device();

		ShaderProgram* prg = shaderCtx.GetProgram(desc.shaderProgram);
		if (!prg)
			return nullptr;

		if (prg->type == PipelineType::Compute)
		{
			GfxComputePipelineDesc pipelineDesc = {};

			pipelineDesc.setShader(prg->shaders[0]);
			pipelineDesc.setPushConstants(prg->constants);

			for (auto* layout : prg->layouts)
				pipelineDesc.addLayout(layout);

			m_pipelines[desc] = gDevice.CreateComputePipeline(pipelineDesc);
		}
		else if (prg->type == PipelineType::Graphics)
		{
			GfxGraphicsPipelineDesc pipelineDesc = {};

			for (auto* shader : prg->shaders)
				pipelineDesc.addShader(shader);

			for (auto* layout : prg->layouts)
				pipelineDesc.addLayout(layout);

			pipelineDesc.setPushConstants(prg->constants);
			pipelineDesc.setInput(prg->input);
			pipelineDesc.setRasterizer(desc.rasterizer);
			pipelineDesc.setDepthStencil(desc.depthStencil);
			pipelineDesc.setBlend(desc.blend);
			pipelineDesc.setDynamicRendering(desc.dynamicRendering);
			pipelineDesc.setFlags(desc.flags);

			for (auto& state : desc.dynamicStates)
				pipelineDesc.addDynamicState(state);

			pipelineDesc.input.topology = desc.topology;

			m_pipelines[desc] = gDevice.CreateGraphicsPipeline(pipelineDesc);
		}

		Log::Terminal(LogType::Debug, "PipelineContext", "{} has been created.", desc.cacheName);

		return m_pipelines[desc].get();
	}

	EngineReport PipelineContext::OnInitialize()
	{
		return EngineReport();
	}

	void PipelineContext::OnFinalize()
	{
		m_pipelines.clear();
	}
}