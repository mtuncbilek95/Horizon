#include "Temporary_ShadowResolvePass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>
#include <Engine/Rendering/Forward+/Temporary_ShadowMapPass.h>

#include <Engine/Rendering/Renderables/RenderableArea.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderableDirectionalLight.h>
#include <Engine/Rendering/Renderables/RenderableMesh.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/Math/Vec4f.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/ComputePipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>
#include <Runtime/RHI/Texture/Texture.h>

namespace Horizon::Rendering::Pass
{
	namespace
	{
		constexpr u32 kGroupSize = 8;

		struct ShadowResolveConstants
		{
			Math::Mat4f invViewProj;
			Math::Mat4f lightViewProj;
			Math::Vec4f cameraPos;
			Math::Vec4f lightDirection;
			Math::Vec2f shadowMapSize;
			f32 shadowWorldSize = 0.0f;
			f32 shadowDepthRange = 0.0f;
			f32 lightSize = 0.0f;
			f32 maxPenumbra = 0.05f;
			f32 depthBias = 0.05f;
			f32 normalBias = 1.5f;
			u32 width = 0;
			u32 height = 0;
			u32 sceneDepthIndex = kInvalid32;
			u32 shadowDepthIndex = kInvalid32;
			u32 outputIndex = kInvalid32;
		};
	}

	b8 ShadowResolvePass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, ShadowResolvePass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc computeCompile = {};
		computeCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/Temporary_ShadowResolvePass.comp.hlsl";
		computeCompile.entryPoint = "CSMain";
		computeCompile.stage = RHI::ShaderStage::Compute;
		computeCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
		List<u8> computeCode = RHI::ShaderCompiler::Compile(computeCompile);

		RHI::ShaderModuleDesc computeModule = {};
		computeModule.stage = RHI::ShaderStage::Compute;
		computeModule.entryPoint = "CSMain";
		computeModule.byteCode = computeCode;
		m_computeShader = pDevice->CreateRHIShaderModule(computeModule);

		if (!m_computeShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ShadowResolvePass shader could not be created");
			return false;
		}

		RHI::ComputePipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pComputeShader = m_computeShader.Get();

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ShadowResolvePass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_sceneDepthView = m_resourceHeap->Allocate();
		m_shadowDepthView = m_resourceHeap->Allocate();
		m_outputView = m_resourceHeap->Allocate();

		return true;
	}

	void ShadowResolvePass::OnFinalize()
	{
		if (m_outputView != kInvalid32)
		{
			m_resourceHeap->Free(m_outputView);
			m_outputView = kInvalid32;
		}

		if (m_shadowDepthView != kInvalid32)
		{
			m_resourceHeap->Free(m_shadowDepthView);
			m_shadowDepthView = kInvalid32;
		}

		if (m_sceneDepthView != kInvalid32)
		{
			m_resourceHeap->Free(m_sceneDepthView);
			m_sceneDepthView = kInvalid32;
		}

		m_pipeline.Reset();
		m_computeShader.Reset();
	}

	void ShadowResolvePass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		RHI::TextureDesc maskDesc = {};
		maskDesc.format = MaskFormat;
		maskDesc.usage = RHI::TextureUsage::Storage | RHI::TextureUsage::Sampled;
		maskDesc.width = pArea->colorImageSize.X();
		maskDesc.height = pArea->colorImageSize.Y();

		graph.Creates<Resource::ShadowMaskRaw>(maskDesc);
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::ShaderResource);
		graph.Reads<Resource::ShadowDepth>(RHI::ResourceState::ShaderResource);
		graph.Writes<Resource::ShadowMaskRaw>(RHI::ResourceState::UnorderedAccess);
	}

	void ShadowResolvePass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FrameMesh* pMeshes = board.Get<Engine::FrameMesh>();
		if (!pCamera || !pMeshes)
			return;

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pMask = GetTexture<Resource::ShadowMaskRaw>();
		RHI::Texture* pShadowDepth = GetTexture<Resource::ShadowDepth>();

		m_resourceHeap->Write(m_sceneDepthView, RHI::DescriptorType::Texture, GetTextureView<Resource::PreDepthOutput>());
		m_resourceHeap->Write(m_shadowDepthView, RHI::DescriptorType::Texture, GetTextureView<Resource::ShadowDepth>());
		m_resourceHeap->Write(m_outputView, RHI::DescriptorType::RWTexture, GetTextureView<Resource::ShadowMaskRaw>());

		const Math::Vec3f lightDirection = ShadowMapPass::ResolveLightDirection(board.TryGet<Engine::RenderableDirectionalLight>());

		ShadowResolveConstants constants = {};
		constants.invViewProj = pCamera->camViewProjMatrix.Inverse();
		constants.lightViewProj = ShadowMapPass::BuildLightViewProj(*pCamera, lightDirection, pShadowDepth->GetWidth());
		constants.cameraPos = { pCamera->camWorldPosition.X(), pCamera->camWorldPosition.Y(), pCamera->camWorldPosition.Z(), 1.0f };
		constants.lightDirection = { lightDirection.X(), lightDirection.Y(), lightDirection.Z(), 0.0f };
		constants.shadowMapSize = { static_cast<f32>(pShadowDepth->GetWidth()), static_cast<f32>(pShadowDepth->GetHeight()) };
		constants.shadowWorldSize = ShadowMapPass::ShadowHalfExtent * 2.0f;
		constants.shadowDepthRange = ShadowMapPass::ShadowHalfDepth * 2.0f;
		constants.width = pMask->GetWidth();
		constants.height = pMask->GetHeight();
		constants.sceneDepthIndex = m_sceneDepthView;
		constants.shadowDepthIndex = m_shadowDepthView;
		constants.outputIndex = m_outputView;

		pCmdList->BindPipeline(m_pipeline.Get());
		pCmdList->SetPushConstants(&constants, sizeof(ShadowResolveConstants));
		pCmdList->DispatchCompute((constants.width + kGroupSize - 1) / kGroupSize, (constants.height + kGroupSize - 1) / kGroupSize, 1);
	}
}
