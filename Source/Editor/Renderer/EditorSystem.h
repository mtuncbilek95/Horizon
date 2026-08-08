#pragma once

#include <Engine/Core/System.h>

#include <Runtime/PAL/Window/Window.h>

namespace Horizon::Engine
{
	class PresentationSystem;
}

namespace Horizon::Editor
{
	class EditorRenderer;

	class ViewRegistry;
	class MenuRegistry;

	class H_EXPORT EditorSystem final : public Engine::System
	{
	public:
		EditorSystem() = default;
		~EditorSystem() = default;

		Engine::AppReport OnAttach(Engine::Application* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(Engine::OrderRules& rules) const final;
		void GetExecutionOrder(Engine::OrderRules& rules) const final;

	private:
		PAL::Window* m_engineWindow = nullptr;
		EditorRenderer* m_editorRenderer = nullptr;

		ViewRegistry* m_viewRegistry = nullptr;
		MenuRegistry* m_menuRegistry = nullptr;

		Engine::PresentationSystem* m_presentationSub = nullptr;
	};
}