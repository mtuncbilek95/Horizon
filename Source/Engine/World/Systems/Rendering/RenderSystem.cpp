#include "RenderSystem.h"

#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Engine/World/Components/Rendering/CameraComponent.h>
#include <Engine/World/Components/Rendering/MeshComponent.h>
#include <Engine/World/Components/PostProcess/FogComponent.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Mesh/MeshAssetStreamer.h>
#include <Engine/Asset/Mesh/MeshVertex.h>

#include <Runtime/Log/Terminal.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>

#include <Runtime/RHI/Shader/GfxShaderCompiler.h>
#include <Runtime/RHI/Shader/GfxShader.h>
#include <Runtime/RHI/Shader/GfxShaderDesc.h>
#include <Runtime/RHI/Pipeline/GfxGraphicsPipelineDesc.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>

#include <Runtime/Math/Mat4f.h>

namespace Horizon::Engine
{
	static constexpr RHI::GfxTextureFormat sFunDepthFormat = RHI::GfxTextureFormat::D32_FLOAT;
	static constexpr u32 sFunHeapCapacity = 16;

	RHI::GfxPipeline* sFunPipeline = nullptr;
	RHI::GfxDescriptorHeap* sFunDepthHeap = nullptr;
	RHI::GfxTexture* sFunDepthTextures[GraphicsContext::MaxFramesInFlight] = {};
	u32 sFunDepthViews[GraphicsContext::MaxFramesInFlight] = {};

	struct FunPushConstant
	{
		Math::Mat4f model;
		Math::Mat4f viewProj;
		Math::Vec3f cameraPos;
		u32 vertexBufferIndex;
		u32 firstVertex;
		f32 fogDensity;
		f32 fogStartDistance;
		f32 fogEndDistance;
		Math::Vec3f fogColor;
		f32 fogMaxOpacity = 0;
		u32 fogMode = 0;
	};

	u32 sFunVertexSrv = kInvalid32;

	static RHI::GfxShader* CreateFunShader(RHI::GfxDevice* pDevice, const std::string& filePath, RHI::GfxShaderStage stage, const std::string& entryPoint, const std::string& debugName)
	{
		List<u8> byteCode = RHI::GfxShaderCompiler::Compile(filePath, stage, entryPoint);

		if (byteCode.IsEmpty())
		{
			Terminal::Error("RenderSystem", "{} could not be compiled with entry point {}", filePath, entryPoint);
			return nullptr;
		}

		RHI::GfxShaderDesc shaderDesc = {};
		shaderDesc.stage = stage;
		shaderDesc.pByteCode = byteCode.GetData();
		shaderDesc.byteCodeSize = byteCode.GetCount();

		RHI::GfxShader* pShader = pDevice->CreateShader(shaderDesc);
		if (pShader == nullptr)
		{
			Terminal::Error("RenderSystem", "{} could not be turned into a shader object", filePath);
			return nullptr;
		}

		pShader->SetDebugName(debugName.data());

		return pShader;
	}

	static RHI::GfxPipeline* CreateFunPipeline(RHI::GfxDevice* pDevice)
	{
		const std::string shaderRoot = std::string(HORIZON_RESOURCE_DIR) + "/Shaders/Testers/";

		RHI::GfxShader* pVertexShader = CreateFunShader(pDevice, shaderRoot + "BasicMesh.vert.hlsl", RHI::GfxShaderStage::Vertex, "VSMain", "BasicForward_Vertex");

		if (pVertexShader == nullptr)
			return nullptr;

		RHI::GfxShader* pPixelShader = CreateFunShader(pDevice, shaderRoot + "BasicMesh.frag.hlsl", RHI::GfxShaderStage::Pixel, "PSMain", "BasicForward_Fragment");

		if (pPixelShader == nullptr)
		{
			Memory::Allocator::Delete(pVertexShader);
			return nullptr;
		}

		RHI::GfxGraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pVertexShader = pVertexShader;
		pipelineDesc.pPixelShader = pPixelShader;
		pipelineDesc.colorFormats[0] = RHI::GfxTextureFormat::RGBA8_UNORM;
		pipelineDesc.colorTargetCount = 1;
		pipelineDesc.depthFormat = sFunDepthFormat;
		pipelineDesc.topology = RHI::GfxPrimitiveTopology::TriangleList;

		pipelineDesc.rasterizer.fillMode = RHI::GfxFillMode::Solid;
		pipelineDesc.rasterizer.cullMode = RHI::GfxCullMode::None;

		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = true;
		pipelineDesc.depthStencil.depthCompare = RHI::GfxCompareOp::Greater;

		RHI::GfxPipeline* pPipeline = pDevice->CreatePipeline(pipelineDesc);
		Memory::Allocator::Delete(pVertexShader);
		Memory::Allocator::Delete(pPixelShader);

		if (pPipeline == nullptr)
		{
			Terminal::Error("RenderSystem", "BasicMesh pipeline could not be created");
			return nullptr;
		}

		pPipeline->SetDebugName("BasicForward_SolidPipeline");

		return pPipeline;
	}

	static RHI::GfxDescriptorHeap* CreateFunHeap(RHI::GfxDevice* pDevice, RHI::GfxDescriptorHeapType type, u32 capacity)
	{
		RHI::GfxDescriptorHeapDesc heapDesc = {};
		heapDesc.type = type;
		heapDesc.capacity = capacity;

		RHI::GfxDescriptorHeap* pHeap = pDevice->CreateDescriptorHeap(heapDesc);
		if (pHeap == nullptr)
			Terminal::Error("RenderSystem", "Descriptor heap type {} with capacity {} could not be created", u32(type), capacity);

		return pHeap;
	}

	static void ClearFunDepthTexture(u32 imageIndex)
	{
		if (sFunDepthTextures[imageIndex] == nullptr)
			return;

		sFunDepthHeap->Free(sFunDepthViews[imageIndex]);
		sFunDepthViews[imageIndex] = kInvalid32;

		Memory::Allocator::Delete(sFunDepthTextures[imageIndex]);
		sFunDepthTextures[imageIndex] = nullptr;
	}

	static b8 RecreateFunDepthTexture(RHI::GfxDevice* pDevice, u32 imageIndex, const Math::Vec2u& size)
	{
		ClearFunDepthTexture(imageIndex);

		RHI::GfxTextureDesc texDesc = {};
		texDesc.width = size.X();
		texDesc.height = size.Y();
		texDesc.type = RHI::GfxTextureType::Tex2D;
		texDesc.format = sFunDepthFormat;
		texDesc.usage = RHI::GfxTextureUsage::DepthStencil;

		RHI::GfxTexture* pTexture = pDevice->CreateTexture(texDesc);
		if (pTexture == nullptr)
		{
			Terminal::Error("RenderSystem", "Scene depth target {}x{} could not be created", size.X(), size.Y());
			return false;
		}

		pTexture->SetDebugName(std::string("Render_DepthTex" + std::to_string(imageIndex)).data());

		sFunDepthViews[imageIndex] = sFunDepthHeap->CreateDepthStencilView(pTexture);
		sFunDepthTextures[imageIndex] = pTexture;

		return true;
	}

	b8 RenderSystem::OnInitialize()
	{
		m_context = GetEngine()->RequestContext<GraphicsContext>();

		if (!m_context)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, render system stays down");
			return false;
		}

		m_device = m_context->GetDevice();
		m_queue = m_context->GetGraphicsQueue();

		m_resourceHeap = CreateFunHeap(m_device, RHI::GfxDescriptorHeapType::Resource, sFunHeapCapacity);
		m_colorHeap = CreateFunHeap(m_device, RHI::GfxDescriptorHeapType::Color, sFunHeapCapacity);
		sFunDepthHeap = CreateFunHeap(m_device, RHI::GfxDescriptorHeapType::Depth, sFunHeapCapacity);

		if (m_resourceHeap == nullptr || m_colorHeap == nullptr || sFunDepthHeap == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Descriptor heaps are unavailable, render system stays down");
			return false;
		}

		ResizeImage({ 1280, 720 });
		m_slots.Resize(GraphicsContext::MaxFramesInFlight);
		for (u32 i = 0; i < GraphicsContext::MaxFramesInFlight; i++)
		{
			if (!RecreateSlot(i))
				return false;
		}

		m_fence = m_device->CreateFence();

		sFunPipeline = CreateFunPipeline(m_device);

		if (sFunPipeline == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Mesh pipeline is unavailable, render system stays down");
			return false;
		}

		return true;
	}

	void RenderSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		Math::Mat4f viewProj = Math::Mat4f::Identity();
		Math::Vec3f camPos = Math::Vec3f::Zero();
		b8 hasView = false;

		currentScene.ForEach<CameraComponent>([&](EntityHandle handl, CameraComponent& camera)
			{
				if (hasView || !camera.m_inUse)
					return;

				if (camera.m_targetScreen.X() < 1.0f || camera.m_targetScreen.Y() < 1.0f)
					return;

				viewProj = camera.m_viewProjection;
				camPos = camera.m_worldPosition;
				ResizeImage({ u32(camera.m_targetScreen.X()), u32(camera.m_targetScreen.Y()) });
				hasView = true;
			});

		RenderSlot& slot = m_slots[m_frameIndex];

		if (slot.currSize != m_targetSize)
		{
			if (!RecreateSlot(m_frameIndex))
			{
				Terminal::Error(StringOps::GetName(this), "Failed to resize color target slot {}", m_frameIndex);
				return;
			}
		}

		m_fence->WaitCPU(slot.fenceValue);
		ReleasePending(slot);

		slot.pTargetCmd->Begin();
		slot.pTargetCmd->BindDescriptorHeaps(m_resourceHeap, nullptr);
		{
			RHI::GfxTextureBarrier beginBarrier = {};
			beginBarrier.pTexture = slot.pTargetTexture;
			beginBarrier.before = slot.currState;
			beginBarrier.after = slot.currState = RHI::GfxResourceState::RenderTarget;
			beginBarrier.firstMip = 0;
			beginBarrier.firstSlice = 0;
			beginBarrier.mipCount = 1;
			beginBarrier.sliceCount = 1;
			slot.pTargetCmd->Barrier(&beginBarrier, 1);
		}
		RHI::GfxRenderBeginDesc renderDesc = RHI::GfxRenderBeginDesc()
			.AddColorTarget(slot.pTargetTexture, RHI::GfxLoadOp::Clear, { 0.0f, 0.0f, 0.0f, 1.f })
			.SetDepth(sFunDepthTextures[m_frameIndex], RHI::GfxLoadOp::Clear, 0.0f)
			.SetSize(slot.pTargetTexture->GetDesc().width, slot.pTargetTexture->GetDesc().height);

		slot.pTargetCmd->BeginRendering(renderDesc);
		slot.pTargetCmd->SetScissor({ 0, 0, (i32)slot.pTargetTexture->GetDesc().width, (i32)slot.pTargetTexture->GetDesc().height });
		slot.pTargetCmd->SetViewport({ 0, 0, (f32)slot.pTargetTexture->GetDesc().width, (f32)slot.pTargetTexture->GetDesc().height, 0.f, 1.f });
		slot.pTargetCmd->BindPipeline(sFunPipeline);

		AssetService* pAssetService = GetEngine()->RequestService<AssetService>();
		auto* pMeshStreamer = static_cast<MeshAssetStreamer*>(pAssetService->FindStreamer(Reflect::TypeOf<MeshAsset>()));

		if (sFunVertexSrv == kInvalid32)
			sFunVertexSrv = m_resourceHeap->CreateShaderView(pMeshStreamer->GetVertexBuffer());

		slot.pTargetCmd->BindIndexBuffer(pMeshStreamer->GetIndexBuffer(), RHI::GfxIndexType::Index32);
		
		FunPushConstant constants = {};
		b8 hasFog = false;
		currentScene.ForEach<FogComponent>([&](EntityHandle handl, FogComponent fogComp) 
			{
				if (hasFog)
					return;

				constants.fogDensity = fogComp.m_density;
				constants.fogStartDistance = fogComp.m_startDistance;
				constants.fogEndDistance = fogComp.m_endDistance;
				constants.fogColor = { fogComp.m_color.R(), fogComp.m_color.G(), fogComp.m_color.B() };
				constants.fogMaxOpacity = fogComp.m_maxOpacity;
				constants.fogMode = (u32)fogComp.m_mode + 1;

				hasFog = true;
			});

		currentScene.ForEach<MeshComponent, TransformComponent>([&](EntityHandle handl, MeshComponent& mesh, TransformComponent& worldMat)
			{
				MeshAsset* pAsset = mesh.m_meshHandle.GetAsset();
				const b8 isResolved = pAsset != nullptr && pAsset->GetPhysicalEntry().assetId == mesh.m_meshHandle.GetId();
				const b8 isWanted = isResolved && !mesh.m_hideInRender;

				if (mesh.m_permit.IsValid() && (!isWanted || mesh.m_permit.pAsset != pAsset))
				{
					mesh.m_permit.pAsset->EndUse(mesh.m_permit);
					mesh.m_permit = {};
				}

				if (!isWanted)
					return;

				if (!mesh.m_permit.IsValid())
				{
					mesh.m_permit = pAsset->BeginUse();

					if (!mesh.m_permit.IsValid())
					{
						if (pAsset->GetResidencyState() == AssetResidency::Unloaded)
							pAsset->LoadAsync();

						return;
					}
				}

				const PermittedMeshData& permit = mesh.m_permit;

				constants.viewProj = viewProj;
				constants.model = worldMat.m_worldMatrix;
				constants.cameraPos = camPos;
				constants.vertexBufferIndex = sFunVertexSrv;
				slot.pTargetCmd->SetGraphicsConstants(&constants, sizeof(FunPushConstant) / sizeof(u32));

				for (u32 i = 0; i < permit.subMeshCount; ++i)
				{
					const MeshSubMesh& subMesh = permit.pSubMeshes[i];
					const u32 firstVertex = permit.firstVertex + subMesh.vertexOffset;

					slot.pTargetCmd->SetGraphicsConstants(&firstVertex, 1, offsetof(FunPushConstant, firstVertex) / sizeof(u32));
					slot.pTargetCmd->DrawIndexed(subMesh.indexCount, 1, permit.firstIndex + subMesh.indexOffset, 0);
				}
			});

		slot.pTargetCmd->EndRendering();
		{
			RHI::GfxTextureBarrier endBarrier = {};
			endBarrier.pTexture = slot.pTargetTexture;
			endBarrier.before = slot.currState;
			endBarrier.after = slot.currState = RHI::GfxResourceState::ShaderResource;
			endBarrier.firstMip = 0;
			endBarrier.firstSlice = 0;
			endBarrier.mipCount = 1;
			endBarrier.sliceCount = 1;
			slot.pTargetCmd->Barrier(&endBarrier, 1);
		}

		slot.pTargetCmd->End();
		m_queue->Submit(&slot.pTargetCmd, 1);

		slot.fenceValue = m_queue->Signal(m_fence);
		m_frameIndex = (m_frameIndex + 1) % GraphicsContext::MaxFramesInFlight;
	}

	void RenderSystem::OnFinalize()
	{
		m_device->WaitIdle();

		for (usize i = 0; i < m_slots.GetCount(); i++)
			m_slots[i].pendingReleases.Clear();

		if (sFunPipeline)
		{
			Memory::Allocator::Delete(sFunPipeline);
			sFunPipeline = nullptr;
		}

		Memory::Allocator::Delete(m_fence);

		for (usize i = 0; i < m_slots.GetCount(); i++)
		{
			ClearSlot(i);
			Memory::Allocator::Delete(m_slots[i].pTargetCmd);
		}

		if (m_colorHeap)
		{
			Memory::Allocator::Delete(m_colorHeap);
			m_colorHeap = nullptr;
		}

		if (m_resourceHeap)
		{
			if (sFunVertexSrv != kInvalid32)
			{
				m_resourceHeap->Free(sFunVertexSrv);
				sFunVertexSrv = kInvalid32;
			}

			Memory::Allocator::Delete(m_resourceHeap);
			m_resourceHeap = nullptr;
		}

		if (sFunDepthHeap)
		{
			Memory::Allocator::Delete(sFunDepthHeap);
			sFunDepthHeap = nullptr;
		}
	}

	void RenderSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
	}

	u64 RenderSystem::GetSceneView() const
	{
		if (!m_slots[m_frameIndex].pTargetTexture)
			return kInvalid64;

		return m_resourceHeap->GetGpuHandle(m_slots[m_frameIndex].shaderView);
	}

	void RenderSystem::ResizeImage(const Math::Vec2u& imgSize)
	{
		u32 width = Math::Max(1u, imgSize.X());
		u32 height = Math::Max(1u, imgSize.Y());

		if (width == m_targetSize.X() || height == m_targetSize.Y())
			return;

		m_targetSize = { width, height };
	}

	b8 RenderSystem::RecreateSlot(u32 imageIndex)
	{
		RenderSlot& slot = m_slots[imageIndex];

		if (slot.currSize == m_targetSize)
		{
			Terminal::Error(StringOps::GetName(this), "It should not be able to come here");
			return false;
		}

		m_device->WaitIdle();

		if (!ClearSlot(imageIndex))
			return false;

		if (slot.pTargetCmd == nullptr)
			slot.pTargetCmd = m_device->CreateCommandList(RHI::GfxQueueType::Graphics);

		RHI::GfxTextureDesc texDesc = {};
		texDesc.width = m_targetSize.X();
		texDesc.height = m_targetSize.Y();
		texDesc.type = RHI::GfxTextureType::Tex2D;
		texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;
		texDesc.usage = RHI::GfxTextureUsage::RenderTarget | RHI::GfxTextureUsage::Sampled;
		texDesc.clearColor = { 0.0f, 0.0f, 0.0f, 1.f };
		texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;

		slot.pTargetTexture = m_device->CreateTexture(texDesc);
		if (slot.pTargetTexture == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Scene color target {}x{} could not be created",
				m_targetSize.X(), m_targetSize.Y());
			return false;
		}

		slot.pTargetTexture->SetDebugName("Scene - RenderTarget");

		slot.renderTargetView = m_colorHeap->CreateRenderTargetView(slot.pTargetTexture);
		slot.shaderView = m_resourceHeap->CreateShaderView(slot.pTargetTexture);

		if (!RecreateFunDepthTexture(m_device, imageIndex, m_targetSize))
			return false;

		slot.currSize = m_targetSize;
		slot.currState = RHI::GfxResourceState::Common;
		slot.fenceValue = 0;

		return true;
	}

	b8 RenderSystem::ClearSlot(u32 imageIndex)
	{
		RenderSlot& slot = m_slots[imageIndex];

		ReleasePending(slot);
		ClearFunDepthTexture(imageIndex);

		if (!slot.pTargetTexture)
			return true;

		m_colorHeap->Free(slot.renderTargetView);
		m_resourceHeap->Free(slot.shaderView);
		slot.renderTargetView = kInvalid32;
		slot.shaderView = kInvalid32;

		Memory::Allocator::Delete(slot.pTargetTexture);
		slot.pTargetTexture = nullptr;

		slot.fenceValue = 0;

		return true;
	}

	void RenderSystem::ReleasePending(RenderSlot& slot)
	{
		for (const PermittedMeshData& permit : slot.pendingReleases)
			permit.pAsset->EndUse(permit);

		slot.pendingReleases.Clear();
	}
}