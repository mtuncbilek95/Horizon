#include "Engine.h"

#include <Engine/Reflection/ReflectionModule.h>

namespace Horizon
{
	Engine::Engine()
	{
		m_reflectionModule = Allocator::Create<ReflectionModule>(CurrLoc());
	}

	Engine::~Engine()
	{
		Allocator::Delete(m_reflectionModule);
	}

	void Engine::Run()
	{
		m_reflectionModule->LoadMainModule();

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

	void Engine::RequestExit(StringView reason)
	{
		m_exitRequested = true;
		Terminal::Info("Engine", "Quit Reason - {}", reason);
	}

}