#include "WindowModule.h"

#include <Engine/Core/Engine.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon
{
	void WindowModule::OnAttach(Engine* pEngine)
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

	void WindowModule::OnSync()
	{
		m_window->PollEvents();

		if (!m_window->GetActive())
			m_engine->RequestExit("Main window is no longer active!");
	}

	void WindowModule::OnDetach()
	{
		Allocator::Delete(m_window);
	}
}