#include "WindowSubsystem.h"

#include <Runtime/PAL/Window/Window.h>

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSubsystem.h>

namespace Horizon
{
	void WindowSubsystem::OnAttach(Engine* pEngine)
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
		m_window->Show();
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

	void WindowSubsystem::GetExecuteAfter(std::vector<std::type_index>& out) const
	{
		Requires<JobSubsystem>(out);
	}

	void WindowSubsystem::GetExecuteBefore(std::vector<std::type_index>& out) const
	{
	}
}