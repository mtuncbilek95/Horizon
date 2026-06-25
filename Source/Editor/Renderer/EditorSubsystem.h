#pragma once

#include <Engine/Core/Subsystem.h>

#include <Runtime/PAL/Window/Window.h>

namespace Horizon
{
	class EditorRenderer;
	class PresentationSubsystem;

	class EditorSubsystem final : public Subsystem
	{
	public:
		EditorSubsystem() = default;
		~EditorSubsystem() = default;

		EngineReport OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		PAL::Window* m_engineWindow = nullptr;
		EditorRenderer* m_editorRenderer = nullptr;

		PresentationSubsystem* m_presentationSub = nullptr;
	};
}