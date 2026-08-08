#include "EditorSystem.h"

#include <Editor/Renderer/EditorRenderer.h>
#include <Editor/Views/ViewRegistry.h>
#include <Editor/MainMenu/MenuRegistry.h>

#include <Engine/Core/Application.h>
#include <Engine/Window/WindowSystem.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationSystem.h>

#include <chrono>

using Clock = std::chrono::high_resolution_clock;

namespace Horizon::Editor
{
	Engine::AppReport EditorSystem::OnAttach(Engine::Application* engine)
	{
		System::OnAttach(engine);

		auto* pWindowSub = m_engine->TryGetSystem<Engine::WindowSystem>();
		if (!pWindowSub)
			return Engine::AppReport("Failed to get WindowSystem. Nothing will work...");

		m_engineWindow = pWindowSub->GetWindow();

		auto* pGraphSub = m_engine->TryGetContext<Engine::GraphicsContext>();
		if (!pGraphSub)
			return Engine::AppReport("Failed to get GraphicsContext. Nothing will work...");

		auto* pOutputSub = m_engine->TryGetSystem<Engine::PresentationSystem>();
		if (!pOutputSub)
			return Engine::AppReport("Failed to get PresentationSystem. Nothing will work...");

		m_presentationSub = pOutputSub;

		EditorRendererDesc renderDesc = {};
		renderDesc.pDevice = pGraphSub->GetDevice();
		renderDesc.pQueue = pGraphSub->GetGraphicsQueue();

		m_editorRenderer = Memory::Allocator::Create<EditorRenderer>(Memory::CurrLoc(), renderDesc);
		Terminal::Debug("EditorSystem", "EditorRenderer has been initialized!");

		m_viewRegistry = Memory::Allocator::Create<ViewRegistry>(Memory::CurrLoc(), m_engine);
		m_viewRegistry->BootstrapViews();

		m_menuRegistry = Memory::Allocator::Create<MenuRegistry>(Memory::CurrLoc());
		m_menuRegistry->BootstrapMenus(m_engine);

		return Engine::AppReport();
	}

	void EditorSystem::OnSync()
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

		// Apply resize window
		// TODO: Later

		i8 imageIndex = m_presentationSub->AcquireImageIndex();
		if (imageIndex == -1)
			return;

		PAL::WindowRect rect = m_engineWindow->GetRect();
		m_editorRenderer->OnResizeWindow(rect.width, rect.height);

		// Render ui work
		m_editorRenderer->BeginRender(deltaTime);

		m_menuRegistry->RenderGUI();
		m_viewRegistry->RenderGUI();

		m_editorRenderer->EndRender(m_presentationSub->GetBackbuffer(imageIndex), imageIndex);
		m_presentationSub->Present(imageIndex);
	}

	void EditorSystem::OnDetach()
	{
		Memory::Allocator::Delete(m_menuRegistry);
		Memory::Allocator::Delete(m_viewRegistry);

		if (m_presentationSub)
			m_presentationSub->WaitIdle();

		Memory::Allocator::Delete(m_editorRenderer);
	}

	void EditorSystem::GetInitializeOrder(Engine::OrderRules& rules) const
	{
		Requires<Engine::PresentationSystem>(rules.after);
	}

	void EditorSystem::GetExecutionOrder(Engine::OrderRules& rules) const
	{
		Requires<Engine::PresentationSystem>(rules.before);
	}
}
