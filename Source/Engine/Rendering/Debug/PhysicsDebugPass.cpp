#include "PhysicsDebugPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderablePhysicsDebug.h>
#include <Engine/Rendering/Debug/PointLightDebugPass.h>
#include <Engine/Rendering/Forward+/ResolvePass.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>
#include <Runtime/RHI/Texture/Texture.h>

#include <cstring>

namespace Horizon::Rendering::Pass
{
	namespace
	{
		constexpr f32 kLineThicknessPixels = 1.5f;
		constexpr u32 kVerticesPerLine = 6;

		struct PhysicsDebugConstants
		{
			Math::Mat4f viewProj;
			u32 vertexBufferIndex;
			f32 thickness;
			f32 invViewportWidth;
			f32 invViewportHeight;
		};
	}

	b8 PhysicsDebugPass::OnInitialize()
	{
		for (u32 i = 0; i < SlotCount; i++)
			m_lineViews[i] = kInvalid32;

		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, PhysicsDebugPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Debug/PhysicsDebugPass.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		vertexCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Debug/PhysicsDebugPass.frag.hlsl";
		fragmentCompile.entryPoint = "PSMain";
		fragmentCompile.stage = RHI::ShaderStage::Pixel;
		fragmentCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
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
			Terminal::Error(StringHelper::GetTypeName(this), "PhysicsDebugPass shaders could not be created");
			return false;
		}

		RHI::ColorTarget colorTarget = {};
		colorTarget.format = Engine::PresentationService::SceneFormat;

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = m_vertexShader.Get();
		pipelineDesc.pPixelShader = m_fragmentShader.Get();
		pipelineDesc.topology = RHI::PrimitiveTopology::TriangleList;
		pipelineDesc.rasterizer.cullMode = RHI::CullMode::None;
		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = false;
		pipelineDesc.depthStencil.depthCompare = RHI::CompareOp::GreaterEqual;
		pipelineDesc.colorTargets.PushBack(colorTarget);
		pipelineDesc.depthFormat = RHI::TextureFormat::D32_FLOAT;
		pipelineDesc.sampleCount = RHI::SampleCount::Count8;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PhysicsDebugPass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();

		RHI::BufferDesc lineDesc = {};
		lineDesc.size = sizeof(Engine::DebugLineVertex) * MaxLineVertexCount;
		lineDesc.usage = RHI::BufferUsage::Storage;
		lineDesc.memory = RHI::MemoryType::Upload;

		for (u32 i = 0; i < SlotCount; i++)
		{
			m_lineBuffers[i] = pDevice->CreateRHIBuffer(lineDesc);
			if (!m_lineBuffers[i] || m_lineBuffers[i]->GetMappedData() == nullptr)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "PhysicsDebugPass line buffer {} could not be created", i);
				return false;
			}

			m_lineViews[i] = m_resourceHeap->Allocate();
			m_resourceHeap->Write(m_lineViews[i], RHI::DescriptorType::Buffer, m_lineBuffers[i].Get(), 0, m_lineBuffers[i]->GetSize(), sizeof(Engine::DebugLineVertex));
		}

		return true;
	}

	void PhysicsDebugPass::OnFinalize()
	{
		for (u32 i = 0; i < SlotCount; i++)
		{
			if (m_lineViews[i] != kInvalid32)
			{
				m_resourceHeap->Free(m_lineViews[i]);
				m_lineViews[i] = kInvalid32;
			}

			m_lineBuffers[i].Reset();
		}

		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();
	}

	void PhysicsDebugPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::DepthRead);
		graph.Writes<Resource::SceneColorMS>(RHI::ResourceState::RenderTarget);

		graph.RunsAfter<PointLightDebugPass>();
		graph.RunsBefore<ResolvePass>();
	}

	void PhysicsDebugPass::Render(const Engine::Blackboard& board)
	{
		const u32 slot = m_slot;
		m_slot = (m_slot + 1) % SlotCount;

		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FramePhysicsDebug* pDebug = board.TryGet<Engine::FramePhysicsDebug>();
		if (!pCamera || !pDebug)
			return;

		u32 vertexCount = static_cast<u32>(std::min<usize>(pDebug->lines.GetCount(), MaxLineVertexCount));
		vertexCount -= vertexCount % 2;
		if (vertexCount == 0)
			return;

		std::memcpy(m_lineBuffers[slot]->GetMappedData(), pDebug->lines.GetData(), sizeof(Engine::DebugLineVertex) * vertexCount);

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::SceneColorMS>();

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::SceneColorMS>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::Load;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachmentCount = 1;

		desc.depthAttachment.pView = GetTextureView<Resource::PreDepthOutput>();
		desc.depthAttachment.loadOp = RHI::LoadOp::Load;
		desc.depthAttachment.storeOp = RHI::StoreOp::Store;
		desc.depthAttachment.readOnly = true;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = static_cast<f32>(pColor->GetWidth());
		viewport.height = static_cast<f32>(pColor->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = static_cast<i32>(pColor->GetWidth());
		scissor.height = static_cast<i32>(pColor->GetHeight());
		pCmdList->SetScissor(scissor);

		pCmdList->BindPipeline(m_pipeline.Get());

		PhysicsDebugConstants constants = {};
		constants.viewProj = pCamera->camViewProjMatrix;
		constants.vertexBufferIndex = m_lineViews[slot];
		constants.thickness = kLineThicknessPixels;
		constants.invViewportWidth = 1.0f / viewport.width;
		constants.invViewportHeight = 1.0f / viewport.height;

		pCmdList->SetPushConstants(&constants, sizeof(PhysicsDebugConstants));
		pCmdList->Draw((vertexCount / 2) * kVerticesPerLine, 1);

		pCmdList->EndRendering();
	}
}