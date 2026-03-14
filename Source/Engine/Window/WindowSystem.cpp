#include "WindowSystem.h"

namespace Horizon
{
	SystemReport WindowSystem::OnInitialize()
	{
		m_window = std::make_unique<BasicWindow>(
			WindowProps()
				.setWindowName("Horizon")
				.setWindowSize({ 1920, 1080 })
				.setWindowMode(WindowMode::Windowed)
		);

		if(!m_window)
			return SystemReport(GetObjectType(), "Failed to create window");

		m_window->Show();
		return SystemReport();
	}

	void WindowSystem::OnSync()
	{
		m_window->ProcessEvents();

		if(!m_window->IsActive())
			GetEngine()->RequestExit("Window closed");
	}

	void WindowSystem::OnFinalize()
	{
		m_window.reset();
	}
}