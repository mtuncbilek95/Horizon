#include "ProceduralSkyPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/Renderables/RenderableArea.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderableDirectionalLight.h>

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

		struct ProceduralSkyConstants
		{
			Math::Mat4f invViewProj;
			f32 toSun[4];
			f32 sunColorIntensity[4];
			u32 outputIndex;
			u32 width;
			u32 height;
			u32 padding;
		};
	}

	b8 ProceduralSkyPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, ProceduralSkyPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc computeCompile = {};
		computeCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ProceduralSkyPass.comp.hlsl";
		computeCompile.entryPoint = "CSMain";
		computeCompile.stage = RHI::ShaderStage::Compute;
		List<u8> computeCode = RHI::ShaderCompiler::Compile(computeCompile);

		RHI::ShaderModuleDesc computeModule = {};
		computeModule.stage = RHI::ShaderStage::Compute;
		computeModule.entryPoint = "CSMain";
		computeModule.byteCode = computeCode;
		m_computeShader = pDevice->CreateRHIShaderModule(computeModule);

		if (!m_computeShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ProceduralSkyPass shader could not be created");
			return false;
		}

		RHI::ComputePipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pComputeShader = m_computeShader.Get();

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ProceduralSkyPass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_outputView = m_resourceHeap->Allocate();

		return true;
	}

	void ProceduralSkyPass::OnFinalize()
	{
		if (m_outputView != kInvalid32)
		{
			m_resourceHeap->Free(m_outputView);
			m_outputView = kInvalid32;
		}

		m_pipeline.Reset();
		m_computeShader.Reset();
	}

	void ProceduralSkyPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		RHI::TextureDesc skyDesc = {};
		skyDesc.format = Engine::PresentationService::SceneFormat;
		skyDesc.usage = RHI::TextureUsage::Storage | RHI::TextureUsage::Sampled;
		skyDesc.width = pArea->colorImageSize.X();
		skyDesc.height = pArea->colorImageSize.Y();

		graph.Creates<Resource::SkyOutput>(skyDesc);
		graph.Writes<Resource::SkyOutput>(RHI::ResourceState::UnorderedAccess);
	}

	void ProceduralSkyPass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		if (!pCamera)
			return;

		const Engine::RenderableDirectionalLight* pDirectional = board.TryGet<Engine::RenderableDirectionalLight>();

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pSky = GetTexture<Resource::SkyOutput>();

		m_resourceHeap->Write(m_outputView, RHI::DescriptorType::RWTexture, GetTextureView<Resource::SkyOutput>());

		ProceduralSkyConstants constants = {};
		constants.invViewProj = pCamera->camViewProjMatrix.Inverse();
		constants.outputIndex = m_outputView;
		constants.width = pSky->GetWidth();
		constants.height = pSky->GetHeight();

		constants.toSun[1] = 1.0f;
		constants.sunColorIntensity[0] = 1.0f;
		constants.sunColorIntensity[1] = 1.0f;
		constants.sunColorIntensity[2] = 1.0f;
		constants.sunColorIntensity[3] = 1.0f;

		if (pDirectional)
		{
			const f32 dirX = pDirectional->direction.X();
			const f32 dirY = pDirectional->direction.Y();
			const f32 dirZ = pDirectional->direction.Z();
			const f32 lengthSq = dirX * dirX + dirY * dirY + dirZ * dirZ;

			if (pDirectional->intensity > 0.0f && lengthSq > 0.0f)
			{
				constants.toSun[0] = -dirX;
				constants.toSun[1] = -dirY;
				constants.toSun[2] = -dirZ;

				constants.sunColorIntensity[0] = pDirectional->color.R();
				constants.sunColorIntensity[1] = pDirectional->color.G();
				constants.sunColorIntensity[2] = pDirectional->color.B();
				constants.sunColorIntensity[3] = pDirectional->intensity;
			}
		}

		pCmdList->BindPipeline(m_pipeline.Get());
		pCmdList->SetPushConstants(&constants, sizeof(ProceduralSkyConstants));
		pCmdList->DispatchCompute((constants.width + kGroupSize - 1) / kGroupSize, (constants.height + kGroupSize - 1) / kGroupSize, 1);
	}
}