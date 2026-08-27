#include "WorldRenderSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/World/World.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	namespace
	{
		constexpr u32 kInitialWidth = 1280;
		constexpr u32 kInitialHeight = 720;
		constexpr RHI::GfxColor kClearColor = { 0.10f, 0.12f, 0.16f, 1.0f };
	}

	b8 WorldRenderSystem::OnInitialize()
	{
		m_graphics = GetEngine()->RequestContext<GraphicsContext>();

		if (!m_graphics)
			return false;

		m_resourceHeap = m_graphics->GetResourceHeap();
		m_colorHeap = m_graphics->GetColorHeap();

		RHI::GfxDevice* pDevice = m_graphics->GetDevice();

		if (!CreateColorTarget(kInitialWidth, kInitialHeight))
			return false;

		m_requestedWidth = kInitialWidth;
		m_requestedHeight = kInitialHeight;

		m_fence = pDevice->CreateFence();

		const u32 frameCount = m_graphics->GetSwapchain()->GetImageCount();

		m_commandLists.Resize(frameCount);
		m_frameValues.Resize(frameCount);

		for (u32 i = 0; i < frameCount; i++)
			m_commandLists[i] = pDevice->CreateCommandList(RHI::GfxQueueType::Graphics);

		return true;
	}

	void WorldRenderSystem::OnExecute(World& world)
	{
		if (m_requestedWidth != m_targetWidth || m_requestedHeight != m_targetHeight)
		{
			m_graphics->GetDevice()->WaitIdle();
			DestroyColorTarget();

			if (!CreateColorTarget(m_requestedWidth, m_requestedHeight))
				return;
		}

		RHI::GfxCommandList* pCommandList = m_commandLists[m_frameIndex];

		m_fence->WaitCPU(m_frameValues[m_frameIndex]);

		pCommandList->Begin();
		pCommandList->BindDescriptorHeaps(m_resourceHeap, nullptr);

		RHI::GfxTextureBarrier toTarget = { m_colorTarget, m_colorState, RHI::GfxResourceState::RenderTarget };

		pCommandList->Barrier(&toTarget, 1);

		RHI::GfxRenderBeginDesc pass = {};

		pass.AddColorTarget(m_colorTarget, RHI::GfxLoadOp::Clear, kClearColor)
			.SetSize(m_targetWidth, m_targetHeight);

		pCommandList->BeginRendering(pass);

		// TODO: Draw the world once the shader and pipeline path exists

		pCommandList->EndRendering();

		RHI::GfxTextureBarrier toShader = { m_colorTarget, RHI::GfxResourceState::RenderTarget,
			RHI::GfxResourceState::ShaderResource };

		pCommandList->Barrier(&toShader, 1);

		m_colorState = RHI::GfxResourceState::ShaderResource;

		pCommandList->End();

		RHI::GfxCommandList* submitList[] = { pCommandList };

		m_graphics->GetGraphicsQueue()->Submit(submitList, 1);

		m_frameValues[m_frameIndex] = m_graphics->GetGraphicsQueue()->Signal(m_fence);
		m_frameIndex = (m_frameIndex + 1) % u32(m_commandLists.GetCount());
	}

	void WorldRenderSystem::OnFinalize()
	{
		m_graphics->GetDevice()->WaitIdle();

		for (RHI::GfxCommandList* pCommandList : m_commandLists)
			Memory::Allocator::Delete(pCommandList);

		Memory::Allocator::Delete(m_fence);

		DestroyColorTarget();
	}

	void WorldRenderSystem::RequestSize(u32 width, u32 height)
	{
		if (width == 0 || height == 0)
			return;

		m_requestedWidth = width;
		m_requestedHeight = height;
	}

	u64 WorldRenderSystem::GetColorTargetHandle() const
	{
		if (!m_colorTarget)
			return 0;

		return m_resourceHeap->GetGpuHandle(m_colorTarget->GetShaderView());
	}

	b8 WorldRenderSystem::CreateColorTarget(u32 width, u32 height)
	{
		RHI::GfxTextureDesc targetDesc = {};

		targetDesc.type = RHI::GfxTextureType::Tex2D;
		targetDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;
		targetDesc.usage = RHI::GfxTextureUsage::RenderTarget | RHI::GfxTextureUsage::Sampled;
		targetDesc.width = width;
		targetDesc.height = height;
		targetDesc.clearColor = kClearColor;

		m_colorTarget = m_graphics->GetDevice()->CreateTexture(targetDesc);

		if (!m_colorTarget)
		{
			Terminal::Error(StringOps::GetName(this), "Scene color target {}x{} could not be created", width, height);
			return false;
		}

		m_colorTarget->SetDebugName("SceneColorTarget");

		m_colorHeap->CreateRenderTargetView(m_colorTarget);
		m_resourceHeap->CreateShaderView(m_colorTarget);

		m_colorState = RHI::GfxResourceState::Common;
		m_targetWidth = width;
		m_targetHeight = height;

		return true;
	}

	void WorldRenderSystem::DestroyColorTarget()
	{
		if (!m_colorTarget)
			return;

		m_resourceHeap->Free(m_colorTarget->GetShaderView());
		m_colorHeap->Free(m_colorTarget->GetRenderTargetView());

		Memory::Allocator::Delete(m_colorTarget);

		m_colorTarget = nullptr;
		m_targetWidth = 0;
		m_targetHeight = 0;
	}
}