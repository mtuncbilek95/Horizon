#include "RenderSystem.h"

#include <Runtime/Log/Terminal.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>

#include <Runtime/RHI/Shader/GfxShaderCompiler.h>
#include <Runtime/RHI/Shader/GfxShader.h>
#include <Runtime/RHI/Shader/GfxShaderDesc.h>
#include <Runtime/RHI/Pipeline/GfxGraphicsPipelineDesc.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>

namespace Horizon::Engine
{
	RHI::GfxShader* pVertexShader = nullptr;
	RHI::GfxShader* pPixelShader = nullptr;
	RHI::GfxPipeline* pTrianglePipeline = nullptr;
	RHI::GfxBuffer* pStorageBuf = nullptr;
	u32 storageView = kInvalid32;

	struct Vertex
	{
		f32 position[3];
		f32 color[4];
	};

	List<Vertex> vertices =
	{
		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } }
	};

	List<u32> indices =
	{
		0, 1, 3, 1, 2, 3
	};

	struct PushConstants
	{
		u32 bufferIndex;
		u32 indexByteOffset;
	};
	PushConstants constants = {};

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

		if (!CreateTexture({ 1280, 720 }))
			return false;

		for (usize i = 0; i < m_context->GetSwapchain()->GetImageCount(); i++)
			m_commandLists.PushBack(m_device->CreateCommandList(RHI::GfxQueueType::Graphics));

		for (usize i = 0; i < m_context->GetSwapchain()->GetImageCount(); i++)
			m_frameValues.PushBack(0);

		m_fence = m_device->CreateFence();

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
		pipelineDesc.rasterizer.cullMode = RHI::GfxCullMode::None;
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

		storageView = m_resourceHeap->CreateShaderView(pStorageBuf);
		return true;
	}

	void RenderSystem::OnExecute(const EngineFrame& ctx)
	{
		if (!EnsureTargets())
			return;

		RHI::GfxCommandList* pCommand = BeginFrame();

		BuildFrameData(ctx);
		RenderScene(pCommand, m_lastImage);
		EndFrame(pCommand);
	}

	void RenderSystem::OnFinalize()
	{
		m_device->WaitIdle();

		m_colorHeap->Recycle();
		m_resourceHeap->Recycle();

		Memory::Allocator::Delete(m_fence);

		for(auto* pCmd : m_commandLists)
			Memory::Allocator::Delete(pCmd);

		Memory::Allocator::Delete(pVertexShader);
		Memory::Allocator::Delete(pPixelShader);
		Memory::Allocator::Delete(pTrianglePipeline);
		Memory::Allocator::Delete(pStorageBuf);

		Memory::Allocator::Delete(m_lastImage);
	}

	u64 RenderSystem::GetSceneView() const
	{
		if (!m_lastImage)
			return kInvalid64;

		return m_resourceHeap->GetGpuHandle(m_lastImage->GetShaderView());
	}

	void RenderSystem::ResizeImage(const Math::Vec2u& imgSize)
	{
		if (imgSize.X() < 1 || imgSize.Y() < 1)
			return;

		m_workableArea = imgSize;
	}

	b8 RenderSystem::CreateTexture(const Math::Vec2u& imgSize)
	{
		RHI::GfxTextureDesc texDesc = {};
		texDesc.width = imgSize.X();
		texDesc.height = imgSize.Y();
		texDesc.type = RHI::GfxTextureType::Tex2D;
		texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;
		texDesc.usage = RHI::GfxTextureUsage::RenderTarget | RHI::GfxTextureUsage::Sampled;
		texDesc.clearColor = { 0.1f, 0.2f, 0.3f, 1.f };
		texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;
		m_lastImage = m_device->CreateTexture(texDesc);

		if (m_lastImage == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Scene color target {}x{} could not be created",
				imgSize.X(), imgSize.Y());
			return false;
		}

		m_lastImage->SetDebugName("Scene - RenderTarget");

		m_colorHeap->CreateRenderTargetView(m_lastImage);
		m_resourceHeap->CreateShaderView(m_lastImage);

		m_workableArea = imgSize;
		m_imageState = RHI::GfxResourceState::Common;

		return true;
	}

	void RenderSystem::DestroyTexture()
	{
		if (m_lastImage == nullptr)
			return;

		Memory::Allocator::Delete(m_lastImage);

		m_lastImage = nullptr;
		m_workableArea = Math::Vec2u::Zero();
	}

	b8 RenderSystem::EnsureTargets()
	{
		if (!m_lastImage)
			return false;

		if (Math::Vec2u(m_lastImage->GetDesc().width, m_lastImage->GetDesc().height) != m_workableArea)
		{
			m_device->WaitIdle();

			DestroyTexture();
			return CreateTexture(m_workableArea);
		}

		return true;
	}

	RHI::GfxCommandList* RenderSystem::BeginFrame()
	{
		m_colorHeap->Recycle();
		m_resourceHeap->Recycle();

		RHI::GfxCommandList* pCommand = m_commandLists[m_frameIndex];

		m_fence->WaitCPU(m_frameValues[m_frameIndex]);

		pCommand->Begin();
		pCommand->BindDescriptorHeaps(m_resourceHeap, nullptr);

		return pCommand;
	}

	void RenderSystem::BuildFrameData(const EngineFrame& ctx)
	{
		constants.bufferIndex = pStorageBuf->GetShaderView();
		constants.indexByteOffset = u32(sizeof(Vertex) * vertices.GetCount());
	}

	void RenderSystem::RenderScene(RHI::GfxCommandList* pCommand, RHI::GfxTexture* pTarget)
	{
		{
			RHI::GfxTextureBarrier beginBarrier = {};
			beginBarrier.pTexture = pTarget;
			beginBarrier.before = m_imageState;
			beginBarrier.after = m_imageState = RHI::GfxResourceState::RenderTarget;
			beginBarrier.firstMip = 0;
			beginBarrier.firstSlice = 0;
			beginBarrier.mipCount = 1;
			beginBarrier.sliceCount = 1;
			pCommand->Barrier(&beginBarrier, 1);
		}
		RHI::GfxRenderBeginDesc renderDesc = RHI::GfxRenderBeginDesc()
			.AddColorTarget(pTarget, RHI::GfxLoadOp::Clear, { 0.1f, 0.2f, 0.3f, 1.f })
			.SetSize(pTarget->GetDesc().width, pTarget->GetDesc().height);

		pCommand->BeginRendering(renderDesc);
		pCommand->SetGraphicsConstants(&constants, 2);

		pCommand->BindPipeline(pTrianglePipeline);
		pCommand->SetScissor({ 0, 0, (i32)pTarget->GetDesc().width, (i32)pTarget->GetDesc().height });
		pCommand->SetViewport({ 0, 0, (f32)pTarget->GetDesc().width, (f32)pTarget->GetDesc().height, 0.f, 1.f });
		pCommand->Draw(indices.GetCount(), 1);

		pCommand->EndRendering();
		{
			RHI::GfxTextureBarrier endBarrier = {};
			endBarrier.pTexture = pTarget;
			endBarrier.before = m_imageState;
			endBarrier.after = m_imageState = RHI::GfxResourceState::ShaderResource;
			endBarrier.firstMip = 0;
			endBarrier.firstSlice = 0;
			endBarrier.mipCount = 1;
			endBarrier.sliceCount = 1;
			pCommand->Barrier(&endBarrier, 1);
		}
	}

	void RenderSystem::EndFrame(RHI::GfxCommandList* pCommand)
	{
		pCommand->End();
		m_queue->Submit(&pCommand, 1);

		m_frameValues[m_frameIndex] = m_queue->Signal(m_fence);
		m_frameIndex = (m_frameIndex + 1) % u32(m_commandLists.GetCount());
	}
}