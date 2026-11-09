#include "ForwardPass.h"

#include <Engine/Asset/Mesh/MeshAssetStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>
#include <Runtime/RHI/Texture/Texture.h>

namespace Horizon::Rendering::Pass
{
	namespace
	{
		struct DrawData
		{
			Math::Mat4f world;
			u32 firstVertex;
		};

		struct DrawIndexedArgs
		{
			u32 indexCount;
			u32 instanceCount;
			u32 firstIndex;
			i32 baseVertex;
			u32 firstInstance;
		};

		struct ForwardConstants
		{
			Math::Mat4f viewProj;
			u32 drawBufferIndex;
			u32 vertexBufferIndex;
		};
	}

	b8 ForwardPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, ForwardPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();
		m_resourceHeap = pContext->GetResourceHeap();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ForwardPass.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ForwardPass.frag.hlsl";
		fragmentCompile.entryPoint = "PSMain";
		fragmentCompile.stage = RHI::ShaderStage::Pixel;
		List<u8> fragmentCode = RHI::ShaderCompiler::Compile(fragmentCompile);

		RHI::ShaderModuleDesc vertexModule = {};
		vertexModule.stage = RHI::ShaderStage::Vertex;
		vertexModule.entryPoint = "VSMain";
		vertexModule.byteCode = vertexCode;
		m_vertexShader = pDevice->CreateRHIShaderModule(vertexModule);

		RHI::ShaderModuleDesc fragmentModule = {};
		fragmentModule.stage = RHI::ShaderStage::Pixel;
		fragmentModule.entryPoint = "PSMain";
		fragmentModule.byteCode = fragmentCode;
		m_fragmentShader = pDevice->CreateRHIShaderModule(fragmentModule);

		if (!m_vertexShader || !m_fragmentShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass shaders could not be created");
			return false;
		}

		RHI::VertexBinding drawIdBinding = {};
		drawIdBinding.binding = 0;
		drawIdBinding.stride = sizeof(u32);
		drawIdBinding.inputRate = RHI::VertexInputRate::PerInstance;

		RHI::VertexAttribute drawIdAttribute = {};
		drawIdAttribute.semantic = "DRAW_ID";
		drawIdAttribute.format = RHI::TextureFormat::R32_UINT;
		drawIdAttribute.binding = 0;

		RHI::ColorTarget colorTarget = {};
		colorTarget.format = Engine::PresentationService::SceneFormat;

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = m_vertexShader.Get();
		pipelineDesc.pPixelShader = m_fragmentShader.Get();
		pipelineDesc.vertexLayout.bindings.PushBack(drawIdBinding);
		pipelineDesc.vertexLayout.attributes.PushBack(drawIdAttribute);
		pipelineDesc.rasterizer.cullMode = RHI::CullMode::None;
		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = true;
		pipelineDesc.depthStencil.depthCompare = RHI::CompareOp::Greater;
		pipelineDesc.colorTargets.PushBack(colorTarget);
		pipelineDesc.depthFormat = RHI::TextureFormat::D32_FLOAT;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass pipeline could not be created");
			return false;
		}

		RHI::BufferDesc bufferDesc = {};
		bufferDesc.memory = RHI::MemoryType::Upload;

		bufferDesc.size = (1u << 16) * sizeof(u32);
		bufferDesc.usage = RHI::BufferUsage::Vertex;
		m_drawIdBuffer = pDevice->CreateRHIBuffer(bufferDesc);

		bufferDesc.size = (1u << 16) * sizeof(DrawData);
		bufferDesc.usage = RHI::BufferUsage::Storage;
		m_drawDataBuffer = pDevice->CreateRHIBuffer(bufferDesc);

		bufferDesc.size = (1u << 16) * sizeof(DrawIndexedArgs);
		bufferDesc.usage = RHI::BufferUsage::Indirect;
		m_indirectBuffer = pDevice->CreateRHIBuffer(bufferDesc);

		if (!m_drawIdBuffer || !m_drawDataBuffer || !m_indirectBuffer)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass draw buffers could not be created");
			return false;
		}

		u32* pDrawIds = reinterpret_cast<u32*>(m_drawIdBuffer->GetMappedData());
		for (u32 i = 0; i < (1u << 16); i++)
			pDrawIds[i] = i;

		m_drawDataView = m_resourceHeap->Allocate();
		m_resourceHeap->Write(m_drawDataView, RHI::DescriptorType::Buffer, m_drawDataBuffer.Get(), 0, m_drawDataBuffer->GetSize(), sizeof(DrawData));

		return true;
	}

	void ForwardPass::OnFinalize()
	{
		if (m_drawDataView != kInvalid32)
		{
			m_resourceHeap->Free(m_drawDataView);
			m_drawDataView = kInvalid32;
		}

		m_indirectBuffer.Reset();
		m_drawDataBuffer.Reset();
		m_drawIdBuffer.Reset();

		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();

		m_resourceHeap = nullptr;
	}

	void ForwardPass::Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph)
	{
		RHI::TextureDesc colorDesc = {};
		colorDesc.format = Engine::PresentationService::SceneFormat;
		colorDesc.usage = RHI::TextureUsage::RenderTarget | RHI::TextureUsage::Sampled;
		colorDesc.width = setupContext.colorImageDimension.X();
		colorDesc.height = setupContext.colorImageDimension.Y();

		graph.Creates<Resource::HDRColor>(colorDesc);
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::DepthWrite);
		graph.Writes<Resource::HDRColor>(RHI::ResourceState::RenderTarget);
	}

	void ForwardPass::Render(const Engine::RenderDrawContext& context)
	{
		DrawData* pDraws = reinterpret_cast<DrawData*>(m_drawDataBuffer->GetMappedData());
		DrawIndexedArgs* pArgs = reinterpret_cast<DrawIndexedArgs*>(m_indirectBuffer->GetMappedData());

		Engine::MeshAssetStreamer* pStreamer = nullptr;
		u32 drawCount = 0;

		for (const Engine::RenderableMesh& renderable : context.meshes)
		{
			if (!renderable.visible || drawCount == (1u << 16))
				continue;

			pStreamer = renderable.mesh.pAsset->GetLoader<Engine::MeshAssetStreamer>();

			for (const Engine::MeshAssetSubmesh& submesh : renderable.mesh.submeshes)
			{
				if (drawCount == (1u << 16))
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "Draw limit reached, max: {}", (1u << 16));
					break;
				}

				pDraws[drawCount].world = renderable.worldMatrix;
				pDraws[drawCount].firstVertex = renderable.mesh.firstVertex + submesh.vertexOffset;

				pArgs[drawCount].indexCount = submesh.indexCount;
				pArgs[drawCount].instanceCount = 1;
				pArgs[drawCount].firstIndex = renderable.mesh.firstIndex + submesh.indexOffset;
				pArgs[drawCount].baseVertex = 0;
				pArgs[drawCount].firstInstance = drawCount;

				drawCount++;
			}
		}

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::HDRColor>();

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::HDRColor>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachments[0].clearColor = { 0.0f, 0.0f, 0.0f, 1.f };
		desc.colorAttachmentCount = 1;

		desc.depthAttachment.pView = GetTextureView<Resource::PreDepthOutput>();
		desc.depthAttachment.loadOp = RHI::LoadOp::Clear;
		desc.depthAttachment.storeOp = RHI::StoreOp::Store;
		desc.depthAttachment.clearDepth = 0.0f;
		desc.depthAttachment.readOnly = false;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = static_cast<f32>(pColor->GetWidth());
		viewport.height = static_cast<f32>(pColor->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = static_cast<i32>(pColor->GetWidth());
		scissor.height = static_cast<i32>(pColor->GetHeight());
		pCmdList->SetScissor(scissor);

		if (drawCount > 0)
		{
			ForwardConstants constants = {};
			constants.viewProj = context.camViewProjMatrix;
			constants.drawBufferIndex = m_drawDataView;
			constants.vertexBufferIndex = pStreamer->GetVertexBufferView();

			pCmdList->BindPipeline(m_pipeline.Get());
			pCmdList->SetPushConstants(&constants, sizeof(ForwardConstants));
			pCmdList->BindVertexBuffer(0, m_drawIdBuffer.Get());
			pCmdList->BindIndexBuffer(pStreamer->GetIndexBuffer());
			pCmdList->DrawIndirectIndexed(m_indirectBuffer.Get(), 0, drawCount);
		}

		pCmdList->EndRendering();
	}
}