#include "EditorService.h"

#include <Editor/Domain/DomainService.h>
#include <Editor/Models/SelectionModel.h>
#include <Editor/Renderer/EditorRenderer.h>
#include <Editor/Views/ViewRegistry.h>
#include <Editor/MainMenu/MenuRegistry.h>
#include <Editor/Toolbar/ToolBarRegistry.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/WorldService.h>

#include <Runtime/PAL/Console/Console.h>
#include <Runtime/PAL/Timer/Timer.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Queue/GfxQueue.h>

#include <imgui.h>

namespace Horizon::Editor
{
	Engine::ModuleReport EditorService::OnInitialize()
	{
		auto* pWindowSub = GetEngine()->RequestService<Engine::WindowService>();
		if (!pWindowSub)
			return Engine::ModuleReport("Failed to get WindowService. Nothing will work...");

		m_engineWindow = pWindowSub->GetWindow();
		m_engineCursor = pWindowSub->GetCursor();

		auto* pGraphSub = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (!pGraphSub)
			return Engine::ModuleReport("Failed to get GraphicsContext. Nothing will work...");

		m_fence = pGraphSub->GetDevice()->CreateFence();
		m_swapchain = pGraphSub->GetSwapchain();
		m_queue = pGraphSub->GetGraphicsQueue();

		EditorRendererDesc renderDesc = {};
		renderDesc.pDevice = pGraphSub->GetDevice();
		renderDesc.pQueue = pGraphSub->GetGraphicsQueue();
		renderDesc.pResourceHeap = pGraphSub->GetResourceHeap();
		renderDesc.colorFormat = m_swapchain->GetDesc().format;
		renderDesc.pWindowService = pWindowSub;

		m_editorRenderer = new EditorRenderer(renderDesc);
		Terminal::Debug(StringHelper::GetTypeName(this), "EditorRenderer has been initialized!");

		m_editorContext.pEngine = GetEngine();
		m_editorContext.pSelection = new SelectionModel();
		m_editorContext.pWindow = pWindowSub->GetWindow();

		m_viewRegistry = new ViewRegistry();

		m_editorContext.pViews = m_viewRegistry;
		m_viewRegistry->BootstrapViews(&m_editorContext);

		m_menuRegistry = new MenuRegistry();

		m_editorContext.pMenus = m_menuRegistry;
		m_menuRegistry->BootstrapMenus(&m_editorContext);

		m_toolRegistry = new ToolBarRegistry();

		m_editorContext.pTools = m_toolRegistry;
		m_toolRegistry->BootstrapItems(&m_editorContext);

		return Engine::ModuleReport();
	}

	void EditorService::OnExecute(const Engine::EngineFrame& ctx)
	{
		const auto& messages = m_engineWindow->GetMessages();

		b8 onResizeDone = true;
		i32 newWindowWidth = -1, newWindowHeight = -1;
		for (const auto& message : messages)
		{
			switch (message.type)
			{
			case PAL::InputMessageType::Resize:
			{
				newWindowWidth = message.width;
				newWindowHeight = message.height;
				onResizeDone = false;
				break;
			}
			case PAL::InputMessageType::DropFiles:
				m_editorContext.upcomingFiles = message.filePaths;
				break;
			default:
				break;
			}
		}

		m_editorRenderer->UpdatePlatform();

		if (!onResizeDone)
		{
			m_swapchain->Resize(newWindowWidth, newWindowHeight);
			onResizeDone = true;
		}

		if (!m_swapchain->AcquireNextImage(m_fence))
			return;

		m_editorRenderer->BeginRender(ctx.DeltaTime());

		m_menuRegistry->RenderGUI();
		m_toolRegistry->RenderGUI();
		m_viewRegistry->RenderGUI(ctx);

		m_editorRenderer->EndRender(m_swapchain->GetImage(m_swapchain->GetCurrentImageIndex()));

		m_swapchain->Present(m_queue, m_fence);
	}

	void EditorService::OnFinalize()
	{
		const u64 value = m_queue->Signal(m_fence);
		m_fence->WaitCPU(value);

		delete m_fence;

		delete m_menuRegistry;
		delete m_toolRegistry;
		delete m_viewRegistry;
		delete m_editorRenderer;

		delete m_editorContext.pSelection;
	}

	void EditorService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Requires<Engine::WindowService>();
		graph.Requires<Engine::GraphicsContext>();
		graph.Requires<DomainService>();
		graph.Requires<Engine::WorldService>();
	}

	void EditorService::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		if (m_menuRegistry)
			m_menuRegistry->OnLibraryRegistered(library);

		if (m_toolRegistry)
			m_toolRegistry->OnLibraryRegistered(library);

		if (m_viewRegistry)
			m_viewRegistry->OnLibraryRegistered(library);
	}

	void EditorService::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		if (m_viewRegistry)
			m_viewRegistry->OnLibraryUnregistered(library);

		if (m_toolRegistry)
			m_toolRegistry->OnLibraryUnregistered(library);

		if (m_menuRegistry)
			m_menuRegistry->OnLibraryUnregistered(library);
	}
}