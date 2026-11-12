#include "Temporary_ShadowBlurPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>
#include <Engine/Rendering/Forward+/Temporary_ShadowResolvePass.h>

#include <Engine/Rendering/Renderables/RenderableArea.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderableMesh.h>

#include <Runtime/Containers/StringHelper.h>
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

		struct ShadowBlurConstants
		{
			i32 stepX = 0;
			i32 stepY = 0;
			i32 radius = 8;
			f32 depthSharpness = 200.0f;
			u32 width = 0;
			u32 height = 0;
			u32 maskIndex = kInvalid32;
			u32 sceneDepthIndex = kInvalid32;
			u32 outputIndex = kInvalid32;
		};
	}

	b8 ShadowBlurPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, ShadowBlurPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc computeCompile = {};
		computeCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/Temporary_ShadowBlurPass.comp.hlsl";
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
			Terminal::Error(StringHelper::GetTypeName(this), "ShadowBlurPass shader could not be created");
			return false;
		}

		RHI::ComputePipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pComputeShader = m_computeShader.Get();

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ShadowBlurPass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_sceneDepthView = m_resourceHeap->Allocate();
		m_maskRawView = m_resourceHeap->Allocate();
		m_tempWriteView = m_resourceHeap->Allocate();
		m_tempReadView = m_resourceHeap->Allocate();
		m_maskView = m_resourceHeap->Allocate();

		return true;
	}

	void ShadowBlurPass::OnFinalize()
	{
		if (m_maskView != kInvalid32)
		{
			m_resourceHeap->Free(m_maskView);
			m_maskView = kInvalid32;
		}

		if (m_tempReadView != kInvalid32)
		{
			m_resourceHeap->Free(m_tempReadView);
			m_tempReadView = kInvalid32;
		}

		if (m_tempWriteView != kInvalid32)
		{
			m_resourceHeap->Free(m_tempWriteView);
			m_tempWriteView = kInvalid32;
		}

		if (m_maskRawView != kInvalid32)
		{
			m_resourceHeap->Free(m_maskRawView);
			m_maskRawView = kInvalid32;
		}

		if (m_sceneDepthView != kInvalid32)
		{
			m_resourceHeap->Free(m_sceneDepthView);
			m_sceneDepthView = kInvalid32;
		}

		m_pipeline.Reset();
		m_computeShader.Reset();
	}

	void ShadowBlurPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		RHI::TextureDesc maskDesc = {};
		maskDesc.format = ShadowResolvePass::MaskFormat;
		maskDesc.usage = RHI::TextureUsage::Storage | RHI::TextureUsage::Sampled;
		maskDesc.width = pArea->colorImageSize.X();
		maskDesc.height = pArea->colorImageSize.Y();

		graph.Creates<Resource::ShadowBlurTemp>(maskDesc);
		graph.Creates<Resource::ShadowMask>(maskDesc);
		graph.Reads<Resource::ShadowMaskRaw>(RHI::ResourceState::ShaderResource);
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::ShaderResource);
		graph.Writes<Resource::ShadowBlurTemp>(RHI::ResourceState::UnorderedAccess);
		graph.Writes<Resource::ShadowMask>(RHI::ResourceState::UnorderedAccess);
	}

	void ShadowBlurPass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FrameMesh* pMeshes = board.Get<Engine::FrameMesh>();
		if (!pCamera || !pMeshes)
			return;

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pTemp = GetTexture<Resource::ShadowBlurTemp>();

		m_resourceHeap->Write(m_sceneDepthView, RHI::DescriptorType::Texture, GetTextureView<Resource::PreDepthOutput>());
		m_resourceHeap->Write(m_maskRawView, RHI::DescriptorType::Texture, GetTextureView<Resource::ShadowMaskRaw>());
		m_resourceHeap->Write(m_tempWriteView, RHI::DescriptorType::RWTexture, GetTextureView<Resource::ShadowBlurTemp>());
		m_resourceHeap->Write(m_tempReadView, RHI::DescriptorType::Texture, GetTextureView<Resource::ShadowBlurTemp>());
		m_resourceHeap->Write(m_maskView, RHI::DescriptorType::RWTexture, GetTextureView<Resource::ShadowMask>());

		ShadowBlurConstants constants = {};
		constants.width = pTemp->GetWidth();
		constants.height = pTemp->GetHeight();
		constants.sceneDepthIndex = m_sceneDepthView;

		const u32 groupCountX = (constants.width + kGroupSize - 1) / kGroupSize;
		const u32 groupCountY = (constants.height + kGroupSize - 1) / kGroupSize;

		pCmdList->BindPipeline(m_pipeline.Get());

		constants.stepX = 1;
		constants.stepY = 0;
		constants.maskIndex = m_maskRawView;
		constants.outputIndex = m_tempWriteView;
		pCmdList->SetPushConstants(&constants, sizeof(ShadowBlurConstants));
		pCmdList->DispatchCompute(groupCountX, groupCountY, 1);

		{
			RHI::TextureBarrier toSRV = {};
			toSRV.before = RHI::ResourceState::UnorderedAccess;
			toSRV.after = RHI::ResourceState::ShaderResource;
			toSRV.pTexture = pTemp;
			pCmdList->Barrier({ toSRV });
		}

		constants.stepX = 0;
		constants.stepY = 1;
		constants.maskIndex = m_tempReadView;
		constants.outputIndex = m_maskView;
		pCmdList->SetPushConstants(&constants, sizeof(ShadowBlurConstants));
		pCmdList->DispatchCompute(groupCountX, groupCountY, 1);

		{
			RHI::TextureBarrier toUAV = {};
			toUAV.before = RHI::ResourceState::ShaderResource;
			toUAV.after = RHI::ResourceState::UnorderedAccess;
			toUAV.pTexture = pTemp;
			pCmdList->Barrier({ toUAV });
		}
	}
}
