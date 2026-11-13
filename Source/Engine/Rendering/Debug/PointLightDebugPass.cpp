#include "PointLightDebugPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderablePointLight.h>
#include <Engine/Rendering/Forward+/ResolvePass.h>
#include <Engine/Rendering/Forward+/Resources.h>
#include <Engine/Rendering/Forward+/SkyCompositePass.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Buffer/Buffer.h>

namespace Horizon::Rendering::Pass
{
	namespace
	{
		constexpr f32 kBillboardPixelRadius = 12.0f;

		struct PointLightDebugConstants
		{
			Math::Mat4f viewProj;
			u32 pointLightBufferIndex;
			f32 pixelRadius;
			f32 invViewportWidth;
			f32 invViewportHeight;
		};
	}

	b8 PointLightDebugPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, PointLightDebugPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Debug/PointLightDebugPass.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		vertexCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Debug/PointLightDebugPass.frag.hlsl";
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
			Terminal::Error(StringHelper::GetTypeName(this), "PointLightDebugPass shaders could not be created");
			return false;
		}

		RHI::ColorTarget colorTarget = {};
		colorTarget.format = Engine::PresentationService::SceneFormat;

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = m_vertexShader.Get();
		pipelineDesc.pPixelShader = m_fragmentShader.Get();
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
			Terminal::Error(StringHelper::GetTypeName(this), "PointLightDebugPass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_pointLightView = m_resourceHeap->Allocate();

		return true;
	}

	void PointLightDebugPass::OnFinalize()
	{
		if (m_pointLightView != kInvalid32)
		{
			m_resourceHeap->Free(m_pointLightView);
			m_pointLightView = kInvalid32;
		}

		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();
	}

	void PointLightDebugPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		graph.Reads<Resource::PointLightList>(RHI::ResourceState::ShaderResource);
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::DepthRead);
		graph.Writes<Resource::SceneColorMS>(RHI::ResourceState::RenderTarget);

		graph.RunsAfter<SkyCompositePass>();
		graph.RunsBefore<ResolvePass>();
	}

	void PointLightDebugPass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FramePointLight* pLightList = board.TryGet<Engine::FramePointLight>();
		if (!pCamera || !pLightList)
			return;

		const u32 lightCount = static_cast<u32>(std::min<usize>(pLightList->pointLights.GetCount(), Engine::kMaxPointLightCount));
		if (lightCount == 0)
			return;

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::SceneColorMS>();
		RHI::Buffer* pPointLights = GetBuffer<Resource::PointLightList>();

		m_resourceHeap->Write(m_pointLightView, RHI::DescriptorType::Buffer, pPointLights, 0, pPointLights->GetSize(), sizeof(Engine::RenderablePointLight));

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

		PointLightDebugConstants constants = {};
		constants.viewProj = pCamera->camViewProjMatrix;
		constants.pointLightBufferIndex = m_pointLightView;
		constants.pixelRadius = kBillboardPixelRadius;
		constants.invViewportWidth = 1.0f / viewport.width;
		constants.invViewportHeight = 1.0f / viewport.height;

		pCmdList->SetPushConstants(&constants, sizeof(PointLightDebugConstants));
		pCmdList->Draw(6, lightCount);

		pCmdList->EndRendering();
	}
}