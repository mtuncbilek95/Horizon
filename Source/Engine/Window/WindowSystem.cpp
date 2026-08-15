#include "WindowSystem.h"

#include <Engine/Core/Application.h>
#include <Engine/Job/JobContext.h>

namespace Horizon::Engine
{
	AppReport WindowSystem::OnAttach(Application* pEngine)
	{
		System::OnAttach(pEngine);

		PAL::WindowDesc winDesc = {};
		winDesc.width = 1280;
		winDesc.height = 720;
		winDesc.mode = PAL::WindowMode::Windowed;
		winDesc.titleName = "Horizon Engine";

#if defined(HORIZON_DEBUG)
		winDesc.flags = PAL::WindowFlags::EnableDragDrop;
#endif

		m_window = Memory::Allocator::Create<PAL::Window>(Memory::CurrLoc(), winDesc);
		if (!m_window)
			return AppReport("Window has not been initialize. Just kill yourself!");

		m_window->Show();

#if defined(HORIZON_WINDOWS)
		Terminal::Info("WindowSystem", "Win32 based window has been initialized!");
#endif
		return AppReport();
	}

	void WindowSystem::OnSync()
	{
		m_window->PollEvents();

		if (!m_window->GetActive())
			m_engine->RequestExit("Main window is no longer active!");
	}

	void WindowSystem::OnDetach()
	{
		Memory::Allocator::Delete(m_window);
	}

	void WindowSystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void WindowSystem::GetExecutionOrder(OrderRules& rules) const
	{
	}
}