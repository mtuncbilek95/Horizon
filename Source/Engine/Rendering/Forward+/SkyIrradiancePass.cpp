#include "SkyIrradiancePass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/Renderables/RenderableDirectionalLight.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Math/Vec4f.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/ComputePipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>

namespace Horizon::Rendering::Pass
{
	namespace
	{
		constexpr u32 kCoefficientCount = 9;

		struct SkyIrradianceConstants
		{
			Math::Vec4f toSun;
			Math::Vec4f sunColorIntensity;
			u32 outputIndex;
		};
	}

	b8 SkyIrradiancePass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, SkyIrradiancePass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc computeCompile = {};
		computeCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/SkyIrradiancePass.comp.hlsl";
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
			Terminal::Error(StringHelper::GetTypeName(this), "SkyIrradiancePass shader could not be created");
			return false;
		}

		RHI::ComputePipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pComputeShader = m_computeShader.Get();

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "SkyIrradiancePass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_outputView = m_resourceHeap->Allocate();

		return true;
	}

	void SkyIrradiancePass::OnFinalize()
	{
		if (m_outputView != kInvalid32)
		{
			m_resourceHeap->Free(m_outputView);
			m_outputView = kInvalid32;
		}

		m_pipeline.Reset();
		m_computeShader.Reset();
	}

	void SkyIrradiancePass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		RHI::BufferDesc bufferDesc = {};
		bufferDesc.memory = RHI::MemoryType::GpuOnly;
		bufferDesc.size = sizeof(Math::Vec4f) * kCoefficientCount;
		bufferDesc.usage = RHI::BufferUsage::Storage;

		graph.Creates<Resource::SkyIrradiance>(bufferDesc);
		graph.Writes<Resource::SkyIrradiance>(RHI::ResourceState::UnorderedAccess);
	}

	void SkyIrradiancePass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableDirectionalLight* pDirectional = board.TryGet<Engine::RenderableDirectionalLight>();

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Buffer* pIrradiance = GetBuffer<Resource::SkyIrradiance>();

		m_resourceHeap->Write(m_outputView, RHI::DescriptorType::RWBuffer, pIrradiance, 0, pIrradiance->GetSize(), sizeof(Math::Vec4f));

		// Give a default irradiance
		SkyIrradianceConstants constants = {};
		constants.toSun = Math::Vec4f(0.0f, 1.0f, 0.0f, 0.0f);
		constants.sunColorIntensity = Math::Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
		constants.outputIndex = m_outputView;

		// If user added sun, use it.
		if (pDirectional)
		{
			const f32 dirX = pDirectional->direction.X();
			const f32 dirY = pDirectional->direction.Y();
			const f32 dirZ = pDirectional->direction.Z();
			const f32 lengthSq = dirX * dirX + dirY * dirY + dirZ * dirZ;

			if (pDirectional->intensity > 0.0f && lengthSq > 0.0f)
			{
				constants.toSun = Math::Vec4f(-dirX, -dirY, -dirZ, 0.0f);
				constants.sunColorIntensity = Math::Vec4f(pDirectional->color.R(), pDirectional->color.G(), pDirectional->color.B(), pDirectional->intensity);
			}
		}

		pCmdList->BindPipeline(m_pipeline.Get());
		pCmdList->SetPushConstants(&constants, sizeof(SkyIrradianceConstants));
		pCmdList->DispatchCompute(1, 1, 1);
	}
}