#include "OpaqueMeshPass.h"

#include <Engine/Graphics/GraphicsContext.h>
#include <Runtime/RHI/Pipeline/GfxGraphicsPipelineDesc.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>

namespace Horizon::Engine
{
	void OpaqueMeshPass::OnInitialize()
	{
		auto* pContext = GetEngine()->RequestContext<GraphicsContext>();
		auto* pDevice = pContext->GetDevice();

		const std::string shaderRoot = std::string(HORIZON_RESOURCE_DIR) + "/Shaders/Forward+/";

		RHI::GfxShader* pVertexShader = CreateShader(pDevice, shaderRoot + "OpaqueMeshPass.vert.hlsl", RHI::GfxShaderStage::Vertex, "VSMain", "OpaqueMeshPass_Vertex");

		if (pVertexShader == nullptr)
			return;

		RHI::GfxShader* pPixelShader = CreateShader(pDevice, shaderRoot + "OpaqueMeshPass.frag.hlsl", RHI::GfxShaderStage::Pixel, "PSMain", "OpaqueMeshPass_Fragment");

		if (pPixelShader == nullptr)
		{
			Memory::Allocator::Delete(pVertexShader);
			return;
		}

		RHI::GfxGraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pVertexShader = pVertexShader;
		pipelineDesc.pPixelShader = pPixelShader;
		pipelineDesc.colorFormats[0] = RHI::GfxTextureFormat::RGBA8_UNORM;
		pipelineDesc.colorTargetCount = 1;
		pipelineDesc.depthFormat = RHI::GfxTextureFormat::D32_FLOAT;
		pipelineDesc.topology = RHI::GfxPrimitiveTopology::TriangleList;

		pipelineDesc.rasterizer.fillMode = RHI::GfxFillMode::Solid;
		pipelineDesc.rasterizer.cullMode = RHI::GfxCullMode::None;

		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = true;
		pipelineDesc.depthStencil.depthCompare = RHI::GfxCompareOp::Greater;

		m_opaquePipeline = pDevice->CreatePipeline(pipelineDesc);
		Memory::Allocator::Delete(pVertexShader);
		Memory::Allocator::Delete(pPixelShader);
	}

	void OpaqueMeshPass::OnFinalize()
	{
		Memory::Allocator::Delete(m_opaquePipeline);
	}

	void OpaqueMeshPass::RenderFrame(const RenderPassFrame& frame)
	{
		RHI::GfxRenderBeginDesc clearDesc = RHI::GfxRenderBeginDesc()
			.AddColorTarget(frame.pColorTarget, RHI::GfxLoadOp::Clear, { 0.0f, 0.0f, 0.0f, 1.f })
			.SetDepth(frame.pDepthTarget, RHI::GfxLoadOp::Clear, 0.0f)
			.SetSize(frame.targetSize.X(), frame.targetSize.Y());

		frame.pCmdList->BeginRendering(clearDesc);
		frame.pCmdList->SetScissor({ 0, 0, i32(frame.targetSize.X()), i32(frame.targetSize.Y()) });
		frame.pCmdList->SetViewport({ 0, 0, f32(frame.targetSize.X()), f32(frame.targetSize.Y()), 0.f, 1.f });

		frame.pCmdList->BindPipeline(m_opaquePipeline);
		frame.pCmdList->BindIndexBuffer(frame.pIndexBuffer, RHI::GfxIndexType::Index32);

		OpaqueMeshConstants constants = {};
		constants.frameDataIndex = frame.frameDataIndex;
		constants.vertexBufferIndex = frame.vertexBufferIndex;

		for (const RenderableObject& object : frame.graphReadyObjects)
		{
			constants.worldMatrix = object.worldMatrix;
			constants.firstVertex = object.firstVertex;

			frame.pCmdList->SetGraphicsConstants(&constants, sizeof(OpaqueMeshConstants) / sizeof(u32));
			frame.pCmdList->DrawIndexed(object.indexCount, 1, object.firstIndex, 0);
		}

		frame.pCmdList->EndRendering();
	}
}