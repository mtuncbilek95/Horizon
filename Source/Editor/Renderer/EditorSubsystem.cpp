#include "EditorSubsystem.h"

#include <Editor/Renderer/EditorRenderer.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSubsystem.h>
#include <Engine/Graphics/GraphicsSubsystem.h>
#include <Engine/Presentation/PresentationSubsystem.h>

#include <imgui.h>

#include <chrono>

using Clock = std::chrono::high_resolution_clock;

namespace Horizon
{
	EngineReport EditorSubsystem::OnAttach(Engine* engine)
	{
		Subsystem::OnAttach(engine);

		auto* pWindowSub = m_engine->TryGetSubsystem<WindowSubsystem>();
		if (!pWindowSub)
			return EngineReport("Failed to get WindowSubsystem. Nothing will work...");

		m_engineWindow = pWindowSub->GetWindow();

		auto* pGraphSub = m_engine->TryGetSubsystem<GraphicsSubsystem>();
		if (!pGraphSub)
			return EngineReport("Failed to get GraphicsSubsystem. Nothing will work...");

		auto* pOutputSub = m_engine->TryGetSubsystem<PresentationSubsystem>();
		if (!pOutputSub)
			return EngineReport("Failed to get PresentationSubsystem. Nothing will work...");

		m_presentationSub = pOutputSub;

		EditorRendererDesc renderDesc = {};
		renderDesc.pDevice = pGraphSub->GetDevice();
		renderDesc.pQueue = pGraphSub->GetGraphicsQueue();

		m_editorRenderer = Allocator::Create<EditorRenderer>(CurrLoc(), renderDesc);
		Terminal::Debug("EditorSubsystem", "EditorRenderer has been initialized!");

		return EngineReport();
	}

	void EditorSubsystem::OnSync()
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

		ImGui::ShowDemoWindow();

		m_editorRenderer->EndRender(m_presentationSub->GetBackbuffer(imageIndex));
		m_presentationSub->Present(imageIndex);
	}

	void EditorSubsystem::OnDetach()
	{
		Allocator::Delete(m_editorRenderer);
	}

	void EditorSubsystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<PresentationSubsystem>(rules.after);
	}

	void EditorSubsystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<PresentationSubsystem>(rules.before);
	}
}
