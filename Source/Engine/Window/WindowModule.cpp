#include "WindowModule.h"

#include <Engine/Core/Engine.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon
{
	void WindowModule::OnAttach(Engine* pEngine)
	{
		Submodule::OnAttach(pEngine);

		m_window = Allocator::Create<Window>(CurrLoc(), WindowDesc());
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