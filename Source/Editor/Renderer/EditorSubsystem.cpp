#include "EditorSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSubsystem.h>
#include <Engine/Graphics/GraphicsSubsystem.h>
#include <Engine/Presentation/PresentationSubsystem.h>

namespace Horizon
{
	EngineReport EditorSubsystem::OnAttach(Engine* engine)
	{
		Subsystem::OnAttach(engine);

		auto* pWindowSub = m_engine->TryGetSubsystem<WindowSubsystem>();
		if (!pWindowSub)
			return EngineReport("Failed to get WindowSubsystem. Nothing will work...");

		auto* pGraphSub = m_engine->TryGetSubsystem<GraphicsSubsystem>();
		if (!pGraphSub)
			return EngineReport("Failed to get GraphicsSubsystem. Nothing will work...");

		auto* pOutputSub = m_engine->TryGetSubsystem<PresentationSubsystem>();
		if (!pOutputSub)
			return EngineReport("Failed to get PresentationSubsystem. Nothing will work...");


		return EngineReport();
	}

	void EditorSubsystem::OnDetach()
	{
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
