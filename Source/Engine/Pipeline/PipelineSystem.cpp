#include "PipelineSystem.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>

#include <Engine/Asset/AssetSystem.h>
#include <Engine/Graphics/GraphicsSystem.h>

namespace Horizon
{
	SystemReport PipelineSystem::OnInitialize()
	{
		return SystemReport();
	}

	void PipelineSystem::OnSync()
	{
	}

	void PipelineSystem::OnFinalize()
	{
	}

	GfxPipeline* PipelineSystem::GetGraphicsPipeline(const GraphicsPipelineStateDesc& desc)
	{
		u64 hash = GetGraphicsPipelineHash(desc);

		auto it = m_pipelines.find(hash);
		if (it != m_pipelines.end())
			return it->second.get();

		auto& assetSystem = RequestSystem<AssetSystem>();

		/*auto* vertexShader = assetSystem.RequestAsset(std::type_index(typeid(ShaderAsset)), desc.meshShaderId);
		auto* fragmentShader = assetSystem.RequestAsset(std::type_index(typeid(ShaderAsset)), desc.fragmentShaderId);*/

		BlendState blendState;
		blendState.setLogicEnable(false);
		for (u32 i = 0; i < desc.rendering.colorAttachmentFormats.size(); ++i)
		{
			blendState.addAttachment(
				BlendAttachment()
				.setBlendEnable(desc.blendEnable)
				.setColorMask(ColorComponent::All)
			);
		}

		auto newPipeline = RequestSystem<GraphicsSystem>().Device()->CreateGraphicsPipeline(
			GfxGraphicsPipelineDesc()
			.addShader(nullptr)
			.addShader(nullptr)
			.setRasterizer(
				RasterizerState()
				.setCullMode(desc.cullMode)
				.setFaceOrientation(FaceOrientation::CCW)
				.setPolygonMode(desc.fillMode)
				.setDepthBiasEnable(false))
			.setInput(
				InputAssembler()
				.setTopology(PrimitiveTopology::TriangleList))
			.setBlend(blendState)
			.setDepthStencil(
				DepthStencilState()
				.setDepthTestEnable(desc.depthTestEnable)
				.setDepthWriteEnable(desc.depthWriteEnable)
				.setDepthOp(desc.depthCompareOp))
			.setViewport(Viewport().setSize({ 1920.f, 1080.f }))
			.setScissor(Scissor().setExtent({ 1920, 1080 }))
			.addLayout(nullptr)
			.setDynamicRendering(desc.rendering)
			.setFlags(PipelineFlags::DescriptorBuffer)
		);

		m_pipelines[hash] = newPipeline;
		return newPipeline.get();
	}

	u64 PipelineSystem::GetGraphicsPipelineHash(const GraphicsPipelineStateDesc& desc) const
	{
		return 0; //XXH64(&desc, sizeof(GraphicsPipelineStateDesc), 0);
	}
}