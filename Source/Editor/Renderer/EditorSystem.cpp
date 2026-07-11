#include "EditorSystem.h"

#include <Editor/Renderer/EditorRenderer.h>
#include <Editor/Widget/WidgetRegistry.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSystem.h>
#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/Presentation/PresentationSystem.h>

#include <chrono>

using Clock = std::chrono::high_resolution_clock;

namespace Horizon
{
	SystemReport EditorSystem::OnAttach(Engine* engine)
	{
		System::OnAttach(engine);

		auto* pWindowSub = m_engine->TryGetSystem<WindowSystem>();
		if (!pWindowSub)
			return SystemReport("Failed to get WindowSystem. Nothing will work...");

		m_engineWindow = pWindowSub->GetWindow();

		auto* pGraphSub = m_engine->TryGetSystem<GraphicsSystem>();
		if (!pGraphSub)
			return SystemReport("Failed to get GraphicsSystem. Nothing will work...");

		auto* pOutputSub = m_engine->TryGetSystem<PresentationSystem>();
		if (!pOutputSub)
			return SystemReport("Failed to get PresentationSystem. Nothing will work...");

		m_presentationSub = pOutputSub;

		EditorRendererDesc renderDesc = {};
		renderDesc.pDevice = pGraphSub->GetDevice();
		renderDesc.pQueue = pGraphSub->GetGraphicsQueue();

		m_editorRenderer = Allocator::Create<EditorRenderer>(CurrLoc(), renderDesc);
		Terminal::Debug("EditorSystem", "EditorRenderer has been initialized!");

		m_widgetSystem = Allocator::Create<WidgetRegistry>(CurrLoc(), m_engine);
		if (!m_widgetSystem)
			return SystemReport("Failed to create WidgetRegistry");

		m_widgetSystem->Invalidate();

		return SystemReport();
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

		m_widgetSystem->Render();

		m_editorRenderer->EndRender(m_presentationSub->GetBackbuffer(imageIndex), imageIndex);
		m_presentationSub->Present(imageIndex);
	}

	void EditorSystem::OnDetach()
	{
		Allocator::Delete(m_editorRenderer);
	}

	void EditorSystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<PresentationSystem>(rules.after);
	}

	void EditorSystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<PresentationSystem>(rules.before);
	}
}
