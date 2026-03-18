#include "Engine.h"

#include <ranges>

namespace Horizon
{
    void Engine::Run()
    {
        while (!m_exitRequested)
        {
            // First check for pending systems to initialize.
            for (auto& system : m_initPendingSystems)
            {
                if (system->OnInitialize())
                    m_systems[system->GetObjectIndex()] = std::move(system);
            }
            m_initPendingSystems.clear();

            // Second finalize pending systems and remove them from the system map.
            for (auto& system : m_finalizePendingSystems)
            {
                system->OnFinalize();
            }

            // Finally, sync all active systems.
            for (auto& [_, system] : m_systems)
                system->OnSync();
        }

        // Finalize all active systems and clear the system map.
        std::vector<ISystem*> vec;
        vec.reserve(m_systems.size());
        for (auto& [key, val] : m_systems)
            vec.push_back(val.get());

        for (usize it = vec.size(); it > 0; it--)
        {
            vec[it - 1]->OnFinalize();
        }
        m_systems.clear();
    }

    void Engine::RequestExit(std::string_view reason)
    {
        Log::Terminal(LogType::Debug, "Engine::RequestExit", reason);
        m_exitRequested = true;
    }
}