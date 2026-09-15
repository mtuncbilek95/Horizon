#include "RenderSystem.h"

#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/Components/CameraComponent.h>
#include <Engine/World/Components/MeshComponent.h>
#include <Engine/World/Components/LocalToWorldComponent.h>
#include <Engine/World/Components/CameraMatrixComponent.h>

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
	RHI::GfxShader* pVertexShader = nullptr;
	RHI::GfxShader* pPixelShader = nullptr;
	RHI::GfxPipeline* pTrianglePipeline = nullptr;
	RHI::GfxBuffer* pStorageBuf = nullptr;
	RHI::GfxBuffer* pCameraBuf = nullptr;

	struct Vertex
	{
		f32 position[3];
		f32 color[4];
	};

	List<Vertex> vertices =
	{
		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },

		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },

		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },

		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },

		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },

		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } }
	};

	List<u32> indices =
	{
		0, 1, 3, 1, 2, 3,
		4, 5, 7, 5, 6, 7,
		8, 9, 11, 9, 10, 11,
		12, 13, 15, 13, 14, 15,
		16, 17, 19, 17, 18, 19,
		20, 21, 23, 21, 22, 23
	};

	struct PushConstants
	{
		u32 bufferIndex;
		u32 cameraIndex;
		u32 cameraOffset;
		u32 indexOffset;
		f32 deltaTime;
	};
	PushConstants constants = {};

	struct ViewObject
	{
		Math::Mat4f mvp = Math::Mat4f::Identity();
	};
	ViewObject pView;
	u8* pCamMapped = nullptr;

	b8 RenderSystem::OnInitialize()
	{
		m_context = GetEngine()->RequestContext<GraphicsContext>();

		if (!m_context)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, render system stays down");
			return false;
		}

		m_device = m_context->GetDevice();
		m_resourceHeap = m_context->GetResourceHeap();
		m_colorHeap = m_context->GetColorHeap();
		m_queue = m_context->GetGraphicsQueue();

		ResizeImage({ 1280, 720 });
		m_slots.Resize(GraphicsContext::MaxFramesInFlight);
		for (u32 i = 0; i < GraphicsContext::MaxFramesInFlight; i++)
		{
			if (!RecreateSlot(i))
				return false;
		}

		m_fence = m_device->CreateFence();

#pragma region "Temporary Render"
		List<u8> vertexByte = RHI::GfxShaderCompiler::Compile(HORIZON_RESOURCE_DIR + std::string("/Shaders/Testers/Triangle.vert.hlsl"), RHI::GfxShaderStage::Vertex, "VSMain");
		RHI::GfxShaderDesc vertShaderDesc = {};
		vertShaderDesc.pByteCode = vertexByte.GetData();
		vertShaderDesc.byteCodeSize = vertexByte.GetCount();
		vertShaderDesc.stage = RHI::GfxShaderStage::Vertex;
		pVertexShader = m_device->CreateShader(vertShaderDesc);

		List<u8> pixelByte = RHI::GfxShaderCompiler::Compile(HORIZON_RESOURCE_DIR + std::string("/Shaders/Testers/Triangle.frag.hlsl"), RHI::GfxShaderStage::Pixel, "PSMain");
		RHI::GfxShaderDesc pixShaderDesc = {};
		pixShaderDesc.pByteCode = pixelByte.GetData();
		pixShaderDesc.byteCodeSize = pixelByte.GetCount();
		pixShaderDesc.stage = RHI::GfxShaderStage::Pixel;
		pPixelShader = m_device->CreateShader(pixShaderDesc);

		RHI::GfxGraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pVertexShader = pVertexShader;
		pipelineDesc.pPixelShader = pPixelShader;
		pipelineDesc.colorFormats[0] = RHI::GfxTextureFormat::RGBA8_UNORM;
		pipelineDesc.colorTargetCount = 1;
		pipelineDesc.depthFormat = RHI::GfxTextureFormat::Undefined;
		pipelineDesc.topology = RHI::GfxPrimitiveTopology::TriangleList;
		pipelineDesc.rasterizer.cullMode = RHI::GfxCullMode::Back;
		pTrianglePipeline = m_device->CreatePipeline(pipelineDesc);

		RHI::GfxBufferDesc bufDesc = {};
		bufDesc.memory = RHI::GfxMemoryType::GpuUpload;
		bufDesc.size = vertices.GetCount() * sizeof(Vertex) + indices.GetCount() * sizeof(u32);
		bufDesc.stride = 0;
		bufDesc.usage = RHI::GfxBufferUsage::Storage;
		pStorageBuf = m_device->CreateBuffer(bufDesc);

		u8* mapped = (u8*)pStorageBuf->Map();
		std::memcpy(mapped, vertices.GetData(), vertices.GetCount() * sizeof(Vertex));
		std::memcpy(mapped + (vertices.GetCount() * sizeof(Vertex)), indices.GetData(), indices.GetCount() * sizeof(u32));
		m_resourceHeap->CreateShaderView(pStorageBuf);

		RHI::GfxBufferDesc cambufDesc = {};
		cambufDesc.memory = RHI::GfxMemoryType::GpuUpload;
		cambufDesc.size = sizeof(ViewObject) * u32(GraphicsContext::MaxFramesInFlight);
		cambufDesc.stride = 0;
		cambufDesc.usage = RHI::GfxBufferUsage::Storage;
		pCameraBuf = m_device->CreateBuffer(cambufDesc);

		pCamMapped = (u8*)pCameraBuf->Map();
		std::memcpy(pCamMapped, &pView, sizeof(ViewObject));
		m_resourceHeap->CreateShaderView(pCameraBuf);
#pragma endregion

		return true;
	}

	void RenderSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		RenderSlot& slot = m_slots[m_frameIndex];

		// Ensure about correct off screen size
		if (slot.currSize != m_targetSize)
		{
			if (!RecreateSlot(m_frameIndex))
			{
				Terminal::Error(StringOps::GetName(this), "Failed to resize color target slot {}", m_frameIndex);
				return;
			}
		}
		
		const u32 cameraOffset = m_frameIndex * u32(sizeof(ViewObject));

		constants.bufferIndex = pStorageBuf->GetShaderView();
		constants.cameraIndex = pCameraBuf->GetShaderView();
		constants.cameraOffset = cameraOffset;
		constants.indexOffset = u32(sizeof(Vertex) * vertices.GetCount());

		Math::Mat4f viewProj = Math::Mat4f::Identity();
		currentScene.ForEach<CameraMatrixComponent>([&](EntityHandle handl, CameraMatrixComponent& camMatrix)
			{
				viewProj = camMatrix.m_viewProjection;
			});

		currentScene.ForEach<MeshComponent, LocalToWorldComponent>([&](EntityHandle handl, MeshComponent& mesh, LocalToWorldComponent& worldMat)
			{
				const Math::Mat4f mvp = viewProj * worldMat.m_worldMatrix;
				std::memcpy(pCamMapped + cameraOffset, &mvp, sizeof(ViewObject));
			});

		m_colorHeap->Recycle();
		m_resourceHeap->Recycle();
	
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
			.AddColorTarget(slot.pTargetTexture, RHI::GfxLoadOp::Clear, { 0.39f, 0.58f, 0.92f, 1.f })
			.SetSize(slot.pTargetTexture->GetDesc().width, slot.pTargetTexture->GetDesc().height);

		slot.pTargetCmd->BeginRendering(renderDesc);
		slot.pTargetCmd->SetGraphicsConstants(&constants, sizeof(PushConstants) / sizeof(u32));

		slot.pTargetCmd->BindPipeline(pTrianglePipeline);
		slot.pTargetCmd->SetScissor({ 0, 0, (i32)slot.pTargetTexture->GetDesc().width, (i32)slot.pTargetTexture->GetDesc().height });
		slot.pTargetCmd->SetViewport({ 0, 0, (f32)slot.pTargetTexture->GetDesc().width, (f32)slot.pTargetTexture->GetDesc().height, 0.f, 1.f });
		slot.pTargetCmd->Draw(indices.GetCount(), 1);

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

		m_colorHeap->Recycle();
		m_resourceHeap->Recycle();

#pragma region"Temporary Render"
		pCameraBuf->Unmap();
		pCamMapped = nullptr;
		pStorageBuf->Unmap();

		Memory::Allocator::Delete(pCameraBuf);
		Memory::Allocator::Delete(pStorageBuf);
		Memory::Allocator::Delete(pTrianglePipeline);
		Memory::Allocator::Delete(pPixelShader);
		Memory::Allocator::Delete(pVertexShader);
#pragma endregion

		Memory::Allocator::Delete(m_fence);

		for (usize i = 0; i < m_slots.GetCount(); i++)
		{
			ClearSlot(i);
			Memory::Allocator::Delete(m_slots[i].pTargetCmd);
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
		texDesc.clearColor = { 0.39f, 0.58f, 0.92f, 1.f };
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

		slot.currSize = m_targetSize;
		slot.currState = RHI::GfxResourceState::Common;
		slot.fenceValue = 0;

		return true;
	}

	b8 RenderSystem::ClearSlot(u32 imageIndex)
	{
		RenderSlot& slot = m_slots[imageIndex];
		
		if (!slot.pTargetTexture)
			return true;

		Memory::Allocator::Delete(slot.pTargetTexture);
		slot.pTargetTexture = nullptr;

		slot.fenceValue = 0;

		return true;
	}
}