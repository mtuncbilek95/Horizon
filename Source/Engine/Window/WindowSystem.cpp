#include "WindowSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobContext.h>

namespace Horizon
{
	EngineReport WindowSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		PAL::WindowDesc winDesc = {};
		winDesc.width = 512;
		winDesc.height = 512;
		winDesc.mode = PAL::WindowMode::Windowed;
		winDesc.titleName = "Horizon Engine";

#if defined(HORIZON_DEBUG)
		winDesc.flags = PAL::WindowFlags::EnableDragDrop;
#endif

		m_window = Allocator::Create<PAL::Window>(CurrLoc(), winDesc);
		if (!m_window)
			return EngineReport("Window has not been initialize. Just kill yourself!");

		m_window->Show();

#if defined(HORIZON_WINDOWS)
		Terminal::Info("WindowSystem", "Win32 based window has been initialized!");
#endif
		return EngineReport();
	}

	void WindowSystem::OnSync()
	{
		m_window->PollEvents();

		if (!m_window->GetActive())
			m_engine->RequestExit("Main window is no longer active!");
	}

	void WindowSystem::OnDetach()
	{
		Allocator::Delete(m_window);
	}

	void WindowSystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void WindowSystem::GetExecutionOrder(OrderRules& rules) const
	{
	}
}