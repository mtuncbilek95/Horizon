#include "CullClusterPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Renderables/RenderableArea.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderablePointLight.h>
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
	b8 CullClusterPass::OnInitialize()
	{
		m_graphics = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (!m_graphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Cannot reach GraphicsContext");
			return false;
		}

		RHI::Device* pDevice = m_graphics->GetDevice();

		RHI::BufferDesc pointLightDesc = {};
		pointLightDesc.size = sizeof(Engine::RenderablePointLight) * Engine::kMaxPointLightCount;
		pointLightDesc.usage = RHI::BufferUsage::CopySrc;
		pointLightDesc.memory = RHI::MemoryType::Upload;
		m_pointLightBuffer = pDevice->CreateRHIBuffer(pointLightDesc);

		if (!m_pointLightBuffer || m_pointLightBuffer->GetMappedData() == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Cannot create point light stage buffer so fuck off!");
			return false;
		}

		RHI::ShaderCompileDesc cullCompile = {};
		cullCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/CullClusterPass.comp.hlsl";
		cullCompile.entryPoint = "CSMain";
		cullCompile.stage = RHI::ShaderStage::Compute;
		List<u8> cullCode = RHI::ShaderCompiler::Compile(cullCompile);

		RHI::ShaderModuleDesc cullModule = {};
		cullModule.stage = RHI::ShaderStage::Compute;
		cullModule.entryPoint = "CSMain";
		cullModule.byteCode = cullCode;
		m_cullShader = pDevice->CreateRHIShaderModule(cullModule);

		if (!m_cullShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CullClusterPass shaders could not be created");
			return false;
		}

		RHI::ComputePipelineDesc cullPipelineDesc = {};
		cullPipelineDesc.pLayout = m_graphics->GetGlobalLayout();
		cullPipelineDesc.pComputeShader = m_cullShader.Get();

		m_cullPipeline = pDevice->CreateRHIPipeline(cullPipelineDesc);
		if (!m_cullPipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CullClusterPass pipeline could not be created");
			return false;
		}

		RHI::ShaderCompileDesc markCompile = {};
		markCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ClusterMarkPass.comp.hlsl";
		markCompile.entryPoint = "CSMain";
		markCompile.stage = RHI::ShaderStage::Compute;
		List<u8> markCode = RHI::ShaderCompiler::Compile(markCompile);

		RHI::ShaderModuleDesc markModule = {};
		markModule.stage = RHI::ShaderStage::Compute;
		markModule.entryPoint = "CSMain";
		markModule.byteCode = markCode;
		m_clusterMarkShader = pDevice->CreateRHIShaderModule(markModule);

		if (!m_clusterMarkShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ClusterMarkPass shaders could not be created");
			return false;
		}

		RHI::ComputePipelineDesc markPipelineDesc = {};
		markPipelineDesc.pLayout = m_graphics->GetGlobalLayout();
		markPipelineDesc.pComputeShader = m_clusterMarkShader.Get();

		m_markPipeline = pDevice->CreateRHIPipeline(markPipelineDesc);
		if (!m_markPipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ClusterMarkPass pipeline could not be created");
			return false;
		}

		m_resourceHeap = m_graphics->GetResourceHeap();
		m_clusterView = m_resourceHeap->Allocate();
		m_pointLightView = m_resourceHeap->Allocate();
		m_lightGridView = m_resourceHeap->Allocate();
		m_activeClusterView = m_resourceHeap->Allocate();
		m_depthView = m_resourceHeap->Allocate();

		return true;
	}

	void CullClusterPass::OnFinalize()
	{
		if (m_lightGridView != kInvalid32)
		{
			m_resourceHeap->Free(m_lightGridView);
			m_lightGridView = kInvalid32;
		}

		if (m_pointLightView != kInvalid32)
		{
			m_resourceHeap->Free(m_pointLightView);
			m_pointLightView = kInvalid32;
		}

		if (m_clusterView != kInvalid32)
		{
			m_resourceHeap->Free(m_clusterView);
			m_clusterView = kInvalid32;
		}

		if (m_activeClusterView != kInvalid32)
		{
			m_resourceHeap->Free(m_activeClusterView);
			m_activeClusterView = kInvalid32;
		}

		if (m_depthView != kInvalid32)
		{
			m_resourceHeap->Free(m_depthView);
			m_depthView = kInvalid32;
		}

		m_pointLightBuffer.Reset();
		m_markPipeline.Reset();
		m_clusterMarkShader.Reset();
		m_cullPipeline.Reset();
		m_cullShader.Reset();
	}

	void CullClusterPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		u32 clusterCountX = (pArea->colorImageSize.X() + kClusterTileSize - 1) / kClusterTileSize;
		u32 clusterCountY = (pArea->colorImageSize.Y() + kClusterTileSize - 1) / kClusterTileSize;
		u32 clusterCount = clusterCountX * clusterCountY * kClusterCountZ;

		// lightList
		RHI::BufferDesc pointLightDesc = {};
		pointLightDesc.size = sizeof(Engine::RenderablePointLight) * Engine::kMaxPointLightCount;
		pointLightDesc.usage = RHI::BufferUsage::Storage | RHI::BufferUsage::CopyDst;
		pointLightDesc.memory = RHI::MemoryType::GpuOnly;
		graph.Creates<Resource::PointLightList>(pointLightDesc);
		graph.Writes<Resource::PointLightList>(RHI::ResourceState::CopyDst);

		// Grid + indexList
		RHI::BufferDesc gridDesc = {};
		gridDesc.size = sizeof(u32) * clusterCount * (kMaxLightsPerCluster + 1);
		gridDesc.usage = RHI::BufferUsage::Storage;
		gridDesc.memory = RHI::MemoryType::GpuOnly;
		graph.Creates<Resource::ClusteredLightGrid>(gridDesc);
		graph.Writes<Resource::ClusteredLightGrid>(RHI::ResourceState::UnorderedAccess);

		// Active clusters
		RHI::BufferDesc clusterActiveDesc = {};
		clusterActiveDesc.size = sizeof(u32) * clusterCount;
		clusterActiveDesc.usage = RHI::BufferUsage::Storage;
		clusterActiveDesc.memory = RHI::MemoryType::GpuOnly;
		graph.Creates<Resource::ActiveCluster>(clusterActiveDesc);
		graph.Writes<Resource::ActiveCluster>(RHI::ResourceState::UnorderedAccess);

		// Reads
		graph.Reads<Resource::ClusterList>(RHI::ResourceState::ShaderResource);
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::ShaderResource);
	}

	void CullClusterPass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		auto* pArea = board.Get<Engine::RenderableArea>();
		if (!pCamera || !pArea)
			return;

		auto* pCmdList = GetCommandList();
		auto* pPointLights = GetBuffer<Resource::PointLightList>();
		if (!pPointLights)
			return;

		u32 lightCount = 0;
		auto* pLightList = board.TryGet<Engine::FramePointLight>();
		if (pLightList)
			lightCount = static_cast<u32>(std::min<usize>(pLightList->pointLights.GetCount(), Engine::kMaxPointLightCount));

		if (lightCount > 0)
		{
			u64 copySize = sizeof(Engine::RenderablePointLight) * lightCount;
			std::memcpy(m_pointLightBuffer->GetMappedData(), pLightList->pointLights.GetData(), copySize);
			pCmdList->CopyBuffer(m_pointLightBuffer.Get(), 0, pPointLights, 0, copySize);
		}

		// From CopyDestination to ShaderResourceView
		{
			RHI::BufferBarrier toSRV = {};
			toSRV.before = RHI::ResourceState::CopyDst;
			toSRV.after = RHI::ResourceState::ShaderResource;
			toSRV.pBuffer = pPointLights;
			pCmdList->Barrier({ toSRV });
		}

		RHI::Buffer* pClusters = GetBuffer<Resource::ClusterList>();
		RHI::Buffer* pLightGrid = GetBuffer<Resource::ClusteredLightGrid>();
		RHI::Buffer* pActiveCluster = GetBuffer<Resource::ActiveCluster>();

		m_resourceHeap->Write(m_clusterView, RHI::DescriptorType::Buffer, pClusters, 0, pClusters->GetSize(), sizeof(ClusterAABB));
		m_resourceHeap->Write(m_pointLightView, RHI::DescriptorType::Buffer, pPointLights, 0, pPointLights->GetSize(), sizeof(Engine::RenderablePointLight));
		m_resourceHeap->Write(m_lightGridView, RHI::DescriptorType::RWBuffer, pLightGrid, 0, pLightGrid->GetSize(), sizeof(u32));
		m_resourceHeap->Write(m_activeClusterView, RHI::DescriptorType::RWBuffer, pActiveCluster, 0, pActiveCluster->GetSize(), sizeof(u32));
		m_resourceHeap->Write(m_depthView, RHI::DescriptorType::Texture, GetTextureView<Resource::PreDepthOutput>());

		u32 clusterCountX = (pArea->colorImageSize.X() + kClusterTileSize - 1) / kClusterTileSize;
		u32 clusterCountY = (pArea->colorImageSize.Y() + kClusterTileSize - 1) / kClusterTileSize;
		u32 clusterTotal = clusterCountX * clusterCountY * kClusterCountZ;

		MarkClusterConstant markConstants = {};
		markConstants.depthIndex = m_depthView;
		markConstants.activeFlagsIndex = m_activeClusterView;
		markConstants.width = pArea->colorImageSize.X();
		markConstants.height = pArea->colorImageSize.Y();
		markConstants.tileSize = kClusterTileSize;
		markConstants.clusterCountX = clusterCountX;
		markConstants.clusterCountY = clusterCountY;
		markConstants.clusterCountZ = kClusterCountZ;
		markConstants.nearPlane = pCamera->nearPlane;
		markConstants.farPlane = pCamera->farPlane;

		pCmdList->BindPipeline(m_markPipeline.Get());
		pCmdList->SetPushConstants(&markConstants, sizeof(MarkClusterConstant));
		pCmdList->DispatchCompute((markConstants.width + 7) / 8, (markConstants.height + 7) / 8, 1);

		pCmdList->BarrierUav();

		CullClusterConstant cullConstants = {};
		cullConstants.viewMatrix = pCamera->camViewMatrix;
		cullConstants.clusterIndex = m_clusterView;
		cullConstants.pointLightIndex = m_pointLightView;
		cullConstants.lightGridIndex = m_lightGridView;
		cullConstants.lightCount = lightCount;
		cullConstants.clusterCountX = clusterCountX;
		cullConstants.clusterCountY = clusterCountY;
		cullConstants.clusterCountZ = kClusterCountZ;
		cullConstants.maxLightsPerCluster = kMaxLightsPerCluster;
		cullConstants.activeFlagsIndex = m_activeClusterView;

		pCmdList->BindPipeline(m_cullPipeline.Get());
		pCmdList->SetPushConstants(&cullConstants, sizeof(CullClusterConstant));
		pCmdList->DispatchCompute((clusterTotal + 63) / kClusterTileSize, 1, 1);

		// From ShaderResourceView to CopyDestination
		{
			RHI::BufferBarrier toDst = {};
			toDst.before = RHI::ResourceState::ShaderResource;
			toDst.after = RHI::ResourceState::CopyDst;
			toDst.pBuffer = pPointLights;
			pCmdList->Barrier({ toDst });
		}
	}
}