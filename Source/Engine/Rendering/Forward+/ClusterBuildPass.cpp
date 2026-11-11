#include "ClusterBuildPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Renderables/RenderableArea.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Forward+/Resources.h>
#include <Engine/Rendering/Forward+/ClusterInformations.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>

namespace Horizon::Rendering::Pass
{
	b8 ClusterBuildPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, ClusterBuildPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc computeCompile = {};
		computeCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ClusterBuildPass.comp.hlsl";
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
			Terminal::Error(StringHelper::GetTypeName(this), "ClusterBuildPass shaders could not be created");
			return false;
		}

		RHI::ComputePipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pComputeShader = m_computeShader.Get();

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ClusterBuildPass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_clusterView = m_resourceHeap->Allocate();

		return true;
	}

	void ClusterBuildPass::OnFinalize()
	{
		if (m_clusterView != kInvalid32)
		{
			m_resourceHeap->Free(m_clusterView);
			m_clusterView = kInvalid32;
		}

		m_pipeline.Reset();
		m_computeShader.Reset();
	}

	void ClusterBuildPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		u32 clusterCountX = (pArea->colorImageSize.X() + kClusterTileSize - 1) / kClusterTileSize;
		u32 clusterCountY = (pArea->colorImageSize.Y() + kClusterTileSize - 1) / kClusterTileSize;
		u32 clusterCount = clusterCountX * clusterCountY * kClusterCountZ;

		RHI::BufferDesc bufferDesc = {};
		bufferDesc.memory = RHI::MemoryType::GpuOnly;
		bufferDesc.size = sizeof(ClusterAABB) * clusterCount;
		bufferDesc.usage = RHI::BufferUsage::Storage;

		graph.Creates<Resource::ClusterList>(bufferDesc);
		graph.Writes<Resource::ClusterList>(RHI::ResourceState::UnorderedAccess);
	}

	void ClusterBuildPass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		if (!pCamera)
			return;

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Buffer* pClusters = GetBuffer<Resource::ClusterList>();

		m_resourceHeap->Write(m_clusterView, RHI::DescriptorType::RWBuffer, pClusters, 0, pClusters->GetSize(), sizeof(ClusterAABB));

		auto* pArea = board.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		u32 clusterCountX = (pArea->colorImageSize.X() + kClusterTileSize - 1) / kClusterTileSize;
		u32 clusterCountY = (pArea->colorImageSize.Y() + kClusterTileSize - 1) / kClusterTileSize;

		ClusterBuildConstant constants = {};
		constants.inverseProj = pCamera->camProjMatrix.Inverse();
		constants.clusterIndex = m_clusterView;
		constants.width = pArea->colorImageSize.X();
		constants.height = pArea->colorImageSize.Y();
		constants.tileSize = kClusterTileSize;
		constants.clusterIndex = m_clusterView;
		constants.clusterCountX = clusterCountX;
		constants.clusterCountY = clusterCountY;
		constants.clusterCountZ = kClusterCountZ;
		constants.nearPlane = pCamera->nearPlane;
		constants.farPlane = pCamera->farPlane;

		pCmdList->BindPipeline(m_pipeline.Get());
		pCmdList->SetPushConstants(&constants, sizeof(ClusterBuildConstant));
		pCmdList->DispatchCompute(clusterCountX, clusterCountY, kClusterCountZ);
	}
}