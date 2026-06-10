#include "Engine.h"

namespace Horizon
{
	void Engine::Run()
	{
		for (auto& module : m_modules)
			module->OnAttach(*this);

		while (!m_exitRequested)
		{
			for (auto& module : m_modules)
				module->OnSync();
		}

		for (auto it = m_modules.rbegin(); it != m_modules.rend(); ++it)
			(*it)->OnDetach();
	}
}