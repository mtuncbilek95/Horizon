#include "EditorService.h"

#include <Editor/Domain/DomainService.h>
#include <Editor/Models/SelectionModel.h>
#include <Editor/Renderer/EditorOverlay.h>
#include <Editor/Views/ViewRegistry.h>
#include <Editor/MainMenu/MenuRegistry.h>
#include <Editor/Toolbar/ToolBarRegistry.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/WorldService.h>
#include <Engine/Presentation/PresentationService.h>

#include <Runtime/PAL/Console/Console.h>
#include <Runtime/PAL/Timer/Timer.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Swapchain/Swapchain.h>

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

		m_device = pGraphSub->GetDevice();

		m_presentation = GetEngine()->RequestService<Engine::PresentationService>();
		if (!m_presentation)
			return Engine::ModuleReport("Failed to get PresentationService. Nothing will be presented...");

		if (m_presentation->GetMode() != Engine::PresentationMode::Editor)
			return Engine::ModuleReport("PresentationService is not registered in Editor mode");

		EditorOverlayDesc overlayDesc = {};
		overlayDesc.pDevice = m_device;
		overlayDesc.pResourceHeap = pGraphSub->GetResourceHeap();
		overlayDesc.colorFormat = m_presentation->GetSwapchain()->GetFormat();
		overlayDesc.pWindowService = pWindowSub;

		m_editorOverlay = MakeUnique<EditorOverlay>(overlayDesc);
		m_presentation->SetOverlay(m_editorOverlay.Get());
		Terminal::Debug(StringHelper::GetTypeName(this), "EditorOverlay has been initialized!");

		m_editorContext.pEngine = GetEngine();
		m_editorContext.pSelection = MakeUnique<SelectionModel>();
		m_editorContext.pWindow = pWindowSub->GetWindow();

		m_viewRegistry = MakeUnique<ViewRegistry>();

		m_editorContext.pViews = m_viewRegistry.Get();
		m_viewRegistry->BootstrapViews(&m_editorContext);

		m_menuRegistry = MakeUnique<MenuRegistry>();

		m_editorContext.pMenus = m_menuRegistry.Get();
		m_menuRegistry->BootstrapMenus(&m_editorContext);

		m_toolRegistry = MakeUnique<ToolBarRegistry>();

		m_editorContext.pTools = m_toolRegistry.Get();
		m_toolRegistry->BootstrapItems(&m_editorContext);

		return Engine::ModuleReport();
	}

	void EditorService::OnExecute(const Engine::EngineFrame& ctx)
	{
		const auto& messages = m_engineWindow->GetMessages();

		for (const auto& message : messages)
		{
			if (message.type == PAL::InputMessageType::DropFiles)
				m_editorContext.upcomingFiles = message.filePaths;
		}

		m_editorOverlay->UpdatePlatform();
		m_editorOverlay->BeginRender(ctx.DeltaTime());

		m_menuRegistry->RenderGUI();
		m_toolRegistry->RenderGUI();
		m_viewRegistry->RenderGUI(ctx);

		m_editorOverlay->EndRender();
	}

	void EditorService::OnFinalize()
	{
		if (m_presentation)
			m_presentation->SetOverlay(nullptr);

		m_device->WaitIdle();

		m_viewRegistry.Reset();
		m_toolRegistry.Reset();
		m_menuRegistry.Reset();
		m_editorContext.pSelection.Reset();

		m_editorOverlay.Reset();
	}

	void EditorService::DeclareDependencies(Engine::ModuleDependency& graph)
	{
		graph.InitAfter<Engine::WindowService, Engine::GraphicsContext, DomainService, Engine::WorldService,
			Engine::PresentationService>();
		graph.RunBefore<Engine::PresentationService>();
	}
}