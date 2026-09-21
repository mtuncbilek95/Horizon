#include "RenderSystem.h"

#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/Components/CameraComponent.h>
#include <Engine/World/Components/MeshComponent.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Mesh/MeshVertex.h>

#include <Runtime/Log/Terminal.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>

#include <Runtime/RHI/Shader/GfxShaderCompiler.h>
#include <Runtime/RHI/Shader/GfxShader.h>
#include <Runtime/RHI/Shader/GfxShaderDesc.h>
#include <Runtime/RHI/Pipeline/GfxGraphicsPipelineDesc.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>

#include <Runtime/Math/Mat4f.h>

#include <cstddef>
#include <string>

namespace Horizon::Engine
{
	static constexpr RHI::GfxTextureFormat sFunDepthFormat = RHI::GfxTextureFormat::D32_FLOAT;
	static constexpr u32 sFunHeapCapacity = 16;

	RHI::GfxPipeline* sFunPipeline = nullptr;
	RHI::GfxDescriptorHeap* sFunDepthHeap = nullptr;
	RHI::GfxTexture* sFunDepthTextures[GraphicsContext::MaxFramesInFlight] = {};

	static RHI::GfxShader* CreateFunShader(RHI::GfxDevice* pDevice, const std::string& filePath, RHI::GfxShaderStage stage, const std::string& entryPoint)
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
			Terminal::Error("RenderSystem", "{} could not be turned into a shader object", filePath);

		return pShader;
	}

	static RHI::GfxPipeline* CreateFunPipeline(RHI::GfxDevice* pDevice)
	{
		const std::string shaderRoot = std::string(HORIZON_RESOURCE_DIR) + "/Shaders/Testers/";

		RHI::GfxShader* pVertexShader = CreateFunShader(pDevice, shaderRoot + "BasicMesh.vert.hlsl", RHI::GfxShaderStage::Vertex, "VSMain");

		if (pVertexShader == nullptr)
			return nullptr;

		RHI::GfxShader* pPixelShader = CreateFunShader(pDevice, shaderRoot + "BasicMesh.frag.hlsl", RHI::GfxShaderStage::Pixel, "PSMain");

		if (pPixelShader == nullptr)
		{
			Memory::Allocator::Delete(pVertexShader);
			return nullptr;
		}

		RHI::GfxGraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pVertexShader = pVertexShader;
		pipelineDesc.pPixelShader = pPixelShader;

		pipelineDesc.inputLayout
			.AddBinding(0, sizeof(MeshVertex))
			.AddAttribute("POSITION", 0, RHI::GfxTextureFormat::RGBA32_FLOAT, 0, offsetof(MeshVertex, position))
			.AddAttribute("NORMAL", 0, RHI::GfxTextureFormat::RGBA32_FLOAT, 0, offsetof(MeshVertex, normal))
			.AddAttribute("TANGENT", 0, RHI::GfxTextureFormat::RGBA32_FLOAT, 0, offsetof(MeshVertex, tangent))
			.AddAttribute("COLOR", 0, RHI::GfxTextureFormat::RGBA32_FLOAT, 0, offsetof(MeshVertex, color))
			.AddAttribute("TEXCOORD", 0, RHI::GfxTextureFormat::RG32_FLOAT, 0, offsetof(MeshVertex, uv));

		pipelineDesc.colorFormats[0] = RHI::GfxTextureFormat::RGBA8_UNORM;
		pipelineDesc.colorTargetCount = 1;
		pipelineDesc.depthFormat = sFunDepthFormat;
		pipelineDesc.topology = RHI::GfxPrimitiveTopology::TriangleList;

		pipelineDesc.rasterizer.fillMode = RHI::GfxFillMode::Solid;
		pipelineDesc.rasterizer.cullMode = RHI::GfxCullMode::None;

		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = true;
		pipelineDesc.depthStencil.depthCompare = RHI::GfxCompareOp::Less;

		RHI::GfxPipeline* pPipeline = pDevice->CreatePipeline(pipelineDesc);

		Memory::Allocator::Delete(pVertexShader);
		Memory::Allocator::Delete(pPixelShader);

		if (pPipeline == nullptr)
		{
			Terminal::Error("RenderSystem", "BasicMesh pipeline could not be created");
			return nullptr;
		}

		pPipeline->SetDebugName("BasicMesh - Pipeline");

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

		pTexture->SetDebugName("Scene - DepthTarget");
		sFunDepthHeap->CreateDepthStencilView(pTexture);

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
		RenderSlot& slot = m_slots[m_frameIndex];

		if (slot.currSize != m_targetSize)
		{
			if (!RecreateSlot(m_frameIndex))
			{
				Terminal::Error(StringOps::GetName(this), "Failed to resize color target slot {}", m_frameIndex);
				return;
			}
		}

		Math::Mat4f viewProj = Math::Mat4f::Identity();
		currentScene.ForEach<CameraComponent>([&](EntityHandle handl, CameraComponent& camMatrix)
			{
				viewProj = camMatrix.m_viewProjection;
			});

		m_fence->WaitCPU(slot.fenceValue);

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
			.SetDepth(sFunDepthTextures[m_frameIndex], RHI::GfxLoadOp::Clear, 1.0f)
			.SetSize(slot.pTargetTexture->GetDesc().width, slot.pTargetTexture->GetDesc().height);

		slot.pTargetCmd->BeginRendering(renderDesc);
		slot.pTargetCmd->SetScissor({ 0, 0, (i32)slot.pTargetTexture->GetDesc().width, (i32)slot.pTargetTexture->GetDesc().height });
		slot.pTargetCmd->SetViewport({ 0, 0, (f32)slot.pTargetTexture->GetDesc().width, (f32)slot.pTargetTexture->GetDesc().height, 0.f, 1.f });
		slot.pTargetCmd->BindPipeline(sFunPipeline);

		AssetService* pAssetService = GetEngine()->RequestService<AssetService>();

		currentScene.ForEach<MeshComponent, TransformComponent>([&](EntityHandle handl, MeshComponent& mesh, TransformComponent& worldMat)
			{
				AssetHandle<MeshAsset>& meshHandle = mesh.m_meshId;

				if (!meshHandle.GetId().IsValid())
					return;

				if (!meshHandle.GetAsset())
					meshHandle = pAssetService->RequestAsset<MeshAsset>(meshHandle.GetId());

				MeshAsset* pAsset = meshHandle.GetAsset();

				if (!pAsset)
					return;

				Math::Mat4f mvp = viewProj * worldMat.m_worldMatrix;
				slot.pTargetCmd->SetGraphicsConstants(&mvp, sizeof(Math::Mat4f) / sizeof(u32));
				slot.pTargetCmd->BindVertexBuffer(pAsset->GetVertexBuffer(), 0, pAsset->GetVertexStride(), 0);
				slot.pTargetCmd->BindIndexBuffer(pAsset->GetIndexBuffer(), RHI::GfxIndexType::Index32);

				const List<MeshSubMesh>& subMeshes = pAsset->GetSubMeshes();

				for (usize i = 0; i < subMeshes.GetCount(); ++i)
				{
					const MeshSubMesh& subMesh = subMeshes[i];
					slot.pTargetCmd->DrawIndexed(subMesh.indexCount, 1, subMesh.indexOffset, static_cast<i32>(subMesh.vertexOffset));
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
			Memory::Allocator::Delete(m_resourceHeap);
			m_resourceHeap = nullptr;
		}

		if (sFunDepthHeap)
		{
			Memory::Allocator::Delete(sFunDepthHeap);
			sFunDepthHeap = nullptr;
		}
	}

	u64 RenderSystem::GetSceneView() const
	{
		if (!m_slots[m_frameIndex].pTargetTexture)
			return kInvalid64;

		return m_resourceHeap->GetGpuHandle(m_slots[m_frameIndex].pTargetTexture->GetShaderView());
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

		m_colorHeap->CreateRenderTargetView(slot.pTargetTexture);
		m_resourceHeap->CreateShaderView(slot.pTargetTexture);

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

		ClearFunDepthTexture(imageIndex);

		if (!slot.pTargetTexture)
			return true;

		Memory::Allocator::Delete(slot.pTargetTexture);
		slot.pTargetTexture = nullptr;

		slot.fenceValue = 0;

		return true;
	}
}