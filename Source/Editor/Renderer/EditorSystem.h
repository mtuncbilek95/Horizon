#pragma once

#include <Engine/Core/System.h>

#include <Runtime/PAL/Window/Window.h>

namespace Horizon
{
	class EditorRenderer;
	class WidgetRegistry;

	class PresentationSystem;

	class H_EXPORT EditorSystem final : public System
	{
	public:
		EditorSystem() = default;
		~EditorSystem() = default;

		SystemReport OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		PAL::Window* m_engineWindow = nullptr;
		EditorRenderer* m_editorRenderer = nullptr;
		WidgetRegistry* m_widgetSystem = nullptr;

		PresentationSystem* m_presentationSub = nullptr;
	};
}