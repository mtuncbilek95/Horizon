#include "WindowSubsystem.h"

#include <Runtime/PAL/Window/Window.h>

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSubsystem.h>

namespace Horizon
{
	EngineReport WindowSubsystem::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		WindowDesc winDesc = {};
		winDesc.width = 1920;
		winDesc.height = 1080;
		winDesc.mode = WindowMode::Windowed;
		winDesc.titleName = "Horizon Engine";

#if defined(HORIZON_DEBUG)
		winDesc.flags = WindowFlags::EnableDragDrop;
#endif

		m_window = Allocator::Create<Window>(CurrLoc(), winDesc);
		if (!m_window)
			return EngineReport("Window has not been initialize. Just kill yourself!");

		m_window->Show();

		return EngineReport();
	}

	void WindowSubsystem::OnSync()
	{
		m_window->PollEvents();

		if (!m_window->GetActive())
			m_engine->RequestExit("Main window is no longer active!");
	}

	void WindowSubsystem::OnDetach()
	{
		Allocator::Delete(m_window);
	}

	void WindowSubsystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<JobSubsystem>(rules.after);
	}

	void WindowSubsystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<JobSubsystem>(rules.after);
	}

}