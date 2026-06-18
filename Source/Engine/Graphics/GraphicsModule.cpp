#include "GraphicsModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>

namespace Horizon
{
	void GraphicsModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto& windowModule = engine.GetModule<WindowModule>();
		const Window& window = windowModule.GetMainWindow();

		m_device = CreateGfxDevice();

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);

		m_frameContext.Init(m_device.get(), m_graphicsQueue.get());
		m_sceneCommandList = m_device->CreateCommandList(GfxQueueType::Graphics);

		m_sceneWidth = window.GetSize().x;
		m_sceneHeight = window.GetSize().y;

		GfxTextureDesc sceneDesc = {};
		sceneDesc.width = m_sceneWidth;
		sceneDesc.height = m_sceneHeight;
		sceneDesc.format = GfxTextureFormat::RGBA8;
		sceneDesc.usage = GfxTextureUsage::Sampled | GfxTextureUsage::RenderTarget;
		sceneDesc.clearColor = { 0.15f, 0.18f, 0.22f, 1.0f };
		m_sceneColor = m_device->CreateTexture(sceneDesc);
	}

	void GraphicsModule::OnSync()
	{
		m_frameContext.BeginFrame();

		GfxTexture* scene = m_sceneColor.GetRawPtr();

		GfxCommandList* cmd = m_sceneCommandList.get();
		cmd->Begin();
		cmd->SetupBindless();

		GfxTextureBarrier toRender = { scene, scene->GetState(), GfxResourceState::RenderTarget };
		cmd->Barrier(&toRender, 1);
		scene->SetState(GfxResourceState::RenderTarget);

		GfxRenderBeginDesc scenePass = {};
		scenePass.addColorTarget(scene, GfxLoadOp::Clear, scene->GetDesc().clearColor)
			.setSize(m_sceneWidth, m_sceneHeight);
		cmd->BeginRendering(scenePass);

		// TODO: Scene geometry record.

		GfxTextureBarrier toSample = { scene, GfxResourceState::RenderTarget, GfxResourceState::ShaderResource };
		cmd->Barrier(&toSample, 1);
		scene->SetState(GfxResourceState::ShaderResource);

		cmd->End();

		GfxCommandList* lists[] = { cmd };
		m_graphicsQueue->Submit(lists, 1);
	}

	void GraphicsModule::OnDetach()
	{
		m_frameContext.WaitIdle();

		m_sceneCommandList.reset();
		m_sceneColor = {};
	}
}
