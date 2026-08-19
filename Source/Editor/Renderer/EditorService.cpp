#include "EditorService.h"

#include <Editor/Renderer/EditorRenderer.h>
#include <Editor/Views/ViewRegistry.h>
#include <Editor/MainMenu/MenuRegistry.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <chrono>

using Clock = std::chrono::high_resolution_clock;

namespace Horizon::Editor
{
	Engine::ModuleReport EditorService::OnInitialize()
	{
		auto* pWindowSub = GetEngine()->RequestService<Engine::WindowService>();
		if (!pWindowSub)
			return Engine::ModuleReport("Failed to get WindowService. Nothing will work...");

		m_engineWindow = pWindowSub->GetWindow();

		auto* pGraphSub = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (!pGraphSub)
			return Engine::ModuleReport("Failed to get GraphicsContext. Nothing will work...");

		EditorRendererDesc renderDesc = {};
		renderDesc.pDevice = pGraphSub->GetDevice();
		renderDesc.pQueue = pGraphSub->GetGraphicsQueue();

		m_editorRenderer = Memory::Allocator::Create<EditorRenderer>(Memory::CurrLoc(), renderDesc);
		Terminal::Debug("EditorService", "EditorRenderer has been initialized!");

		m_viewRegistry = Memory::Allocator::Create<ViewRegistry>(Memory::CurrLoc(), GetEngine());
		m_viewRegistry->BootstrapViews();

		m_menuRegistry = Memory::Allocator::Create<MenuRegistry>(Memory::CurrLoc());
		m_menuRegistry->BootstrapMenus(GetEngine());

		return Engine::ModuleReport();
	}

	void EditorService::OnExecute()
	{
		static auto lastTime = Clock::now();

		auto currentTime = Clock::now();
		f32 deltaTime = std::chrono::duration<f32>(currentTime - lastTime).count();
		lastTime = currentTime;

		const auto& messages = m_engineWindow->GetMessages();

		i32 newWindowWidth = -1, newWindowHeight = -1;
		for (const auto& message : messages)
		{
			switch (message.type)
			{
			case PAL::InputMessageType::MouseMove:
			{
				m_editorRenderer->OnMousePosition(message.mouseX, message.mouseY);
				break;
			}
			case PAL::InputMessageType::MouseDown:
			{
				m_editorRenderer->OnMouseButtonDown(message.button);
				break;
			}
			case PAL::InputMessageType::MouseUp:
			{
				m_editorRenderer->OnMouseButtonUp(message.button);
				break;
			}
			case PAL::InputMessageType::MouseScroll:
			{
				m_editorRenderer->OnMouseWheel(message.scrollY);
				break;
			}
			case PAL::InputMessageType::KeyDown:
			{
				m_editorRenderer->OnKeyboardDown(message.key);
				break;
			}
			case PAL::InputMessageType::KeyUp:
			{
				m_editorRenderer->OnKeyboardUp(message.key);
				break;
			}
			case PAL::InputMessageType::Char:
			{
				m_editorRenderer->OnKeyboardChar(message.character);
				break;
			}
			case PAL::InputMessageType::Resize:
			{
				newWindowWidth = message.width;
				newWindowHeight = message.height;
				break;
			}
			}
		}
	}

	void EditorService::OnFinalize()
	{
		Memory::Allocator::Delete(m_menuRegistry);
		Memory::Allocator::Delete(m_viewRegistry);
		Memory::Allocator::Delete(m_editorRenderer);
	}

	void EditorService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Requires<Engine::WindowService>();
		graph.Requires<Engine::GraphicsContext>();
	}
}
