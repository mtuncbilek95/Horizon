#include "Engine.h"

namespace Horizon
{
	void Engine::Run()
	{
		for (auto& module : m_modules)
			module->OnAttach(this);

		while (!m_exitRequested)
		{
			for (auto& module : m_modules)
				module->OnSync();
		}

		for (auto it = m_modules.rbegin(); it != m_modules.rend(); ++it)
		{
			(*it)->OnDetach();
			Allocator::Delete((*it));
		}

		m_modules.clear();

		Allocator::ReportLeaks();
	}

	void Engine::RequestExit(std::string_view reason)
	{
		m_exitRequested = true;
		Terminal::Info("Engine", "Quit Reason - {}", reason);
	}

}