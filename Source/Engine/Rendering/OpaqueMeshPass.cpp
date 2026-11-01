#include "OpaqueMeshPass.h"

#include <Engine/Graphics/GraphicsContext.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>

namespace Horizon::Engine
{
	void OpaqueMeshPass::OnInitialize()
	{
		auto* pContext = GetEngine()->RequestContext<GraphicsContext>();
		auto* pDevice = pContext->GetDevice();

		const std::string shaderRoot = std::string(HORIZON_RESOURCE_DIR) + "/Shaders/Forward+/";

		UniquePtr<RHI::ShaderModule> pVertexShader = CreateShader(pDevice, shaderRoot + "OpaqueMeshPass.vert.hlsl", RHI::ShaderStage::Vertex, "VSMain", "OpaqueMeshPass_Vertex");

		if (!pVertexShader)
			return;

		UniquePtr<RHI::ShaderModule> pPixelShader = CreateShader(pDevice, shaderRoot + "OpaqueMeshPass.frag.hlsl", RHI::ShaderStage::Pixel, "PSMain", "OpaqueMeshPass_Fragment");

		if (!pPixelShader)
			return;

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = pVertexShader.Get();
		pipelineDesc.pPixelShader = pPixelShader.Get();
		pipelineDesc.colorTargets.PushBack({ RHI::TextureFormat::RGBA8_UNORM });
		pipelineDesc.depthFormat = RHI::TextureFormat::D32_FLOAT;
		pipelineDesc.topology = RHI::PrimitiveTopology::TriangleList;

		pipelineDesc.rasterizer.fillMode = RHI::FillMode::Solid;
		pipelineDesc.rasterizer.cullMode = RHI::CullMode::None;

		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = true;
		pipelineDesc.depthStencil.depthCompare = RHI::CompareOp::Greater;

		m_opaquePipeline = pDevice->CreateRHIPipeline(pipelineDesc);
	}

	void OpaqueMeshPass::OnFinalize()
	{
		m_opaquePipeline = nullptr;
	}

	void OpaqueMeshPass::RenderFrame(const RenderPassFrame& frame)
	{
		if (!m_opaquePipeline)
			return;

		RHI::RenderingDesc rendering = {};
		rendering.colorAttachments[0].pView = frame.pColorView;
		rendering.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		rendering.colorAttachmentCount = 1;
		rendering.depthAttachment.pView = frame.pDepthView;
		rendering.depthAttachment.loadOp = RHI::LoadOp::Clear;
		rendering.depthAttachment.clearDepth = 0.0f;

		frame.pCmdList->StartRendering(rendering);
		frame.pCmdList->SetScissor({ 0, 0, i32(frame.targetSize.X()), i32(frame.targetSize.Y()) });
		frame.pCmdList->SetViewport({ 0, 0, f32(frame.targetSize.X()), f32(frame.targetSize.Y()), 0.f, 1.f });

		if (frame.pIndexBuffer)
		{
			frame.pCmdList->BindPipeline(m_opaquePipeline.Get());
			frame.pCmdList->BindIndexBuffer(frame.pIndexBuffer, RHI::IndexType::Index32);

			OpaqueMeshConstants constants = {};
			constants.frameDataIndex = frame.frameDataIndex;
			constants.vertexBufferIndex = frame.vertexBufferIndex;

			for (const RenderableObject& object : frame.graphReadyObjects)
			{
				constants.worldMatrix = object.worldMatrix;
				constants.firstVertex = object.firstVertex;

				frame.pCmdList->SetPushConstants(&constants, sizeof(OpaqueMeshConstants));
				frame.pCmdList->DrawIndexed(object.indexCount, 1, object.firstIndex, 0);
			}
		}

		frame.pCmdList->EndRendering();
	}
}
