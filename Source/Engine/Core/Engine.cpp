#include "Engine.h"

#include <ranges>

namespace Horizon
{
	void Engine::Run()
	{
		while (!m_exitRequested)
		{
			// First check for pending contexts to initialize.
			for (auto& ctx : m_initPendingContexts)
			{
				if (ctx->OnInitialize())
					m_contexts[ctx->GetObjectIndex()] = std::move(ctx);
				else
					RequestExit(ctx->GetObjectType() + " has failed initializing!");
			}
			m_initPendingContexts.clear();

			// Second check for pending systems to initialize.
			for (auto& system : m_initPendingSystems)
			{
				if (system->OnInitialize())
					m_systems[system->GetObjectIndex()] = std::move(system);
				else
					RequestExit(system->GetObjectType() + " has failed initializing!");
			}
			m_initPendingSystems.clear();

			// Third finalize pending systems and remove them from the system map.
			for (auto& system : m_finalizePendingSystems)
				system->OnFinalize();

			// Fourth finalize pending ctx and remove them from the ctx map.
			for (auto& ctx : m_finalizePendingContexts)
				ctx->OnFinalize();

			// Finally, sync all active systems.
			for (auto& [_, system] : m_systems)
				system->OnSync();
		}

		// Finalize all active systems and contexts and clear the system map.
		std::vector<ISystem*> syss;
		syss.reserve(m_systems.size());
		for (auto& [key, val] : m_systems)
			syss.push_back(val.get());

		for (usize it = syss.size(); it > 0; it--)
			syss[it - 1]->OnFinalize();
		m_systems.clear();

		std::vector<IContext*> ctxs;
		ctxs.reserve(m_systems.size());
		for (auto& [key, val] : m_contexts)
			ctxs.push_back(val.get());

		for (usize it = ctxs.size(); it > 0; it--)
			ctxs[it - 1]->OnFinalize();
		m_contexts.clear();
	}

	void Engine::RequestExit(std::string_view reason)
	{
		Log::Terminal(LogType::Debug, "Engine::RequestExit", reason);
		m_exitRequested = true;
	}
}