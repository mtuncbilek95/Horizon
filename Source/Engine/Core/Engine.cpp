#include "Engine.h"

#include <unordered_set>

namespace Horizon
{
	Engine::Engine()
	{
	}

	Engine::~Engine()
	{
	}

	void Engine::Run()
	{
		while (!m_exitRequested)
		{
			FlushPending();

			for (Subsystem* system : m_activeSystems)
				system->OnSync();
		}

		Shutdown();
		Allocator::ReportLeaks();
	}

	void Engine::RequestExit(std::string_view reason)
	{
		m_exitRequested = true;
		Terminal::Info("Engine", "Quit Reason - {}", reason);
	}

	void Engine::FlushPending()
	{
		// Check the changes
		b8 changed = !m_removePendingSystems.empty() || !m_initPendingSystems.empty();

		// Detach the fuck out pending deletes
		for (auto* system : m_removePendingSystems)
		{
			system->OnDetach();

			std::erase(m_activeSystems, system);

			for (auto it = m_lookup.begin(); it != m_lookup.end(); it++)
			{
				if (it->second == system)
				{
					m_lookup.erase(it);
					break;
				}
			}

			Allocator::Delete(system);
		}

		// Clean up the mess
		m_removePendingSystems.clear();

		// Attach new shit
		for (auto* system : m_initPendingSystems)
		{
			system->OnAttach(this);
			m_activeSystems.push_back(system);
		}
		m_initPendingSystems.clear();

		if (changed)
			SortActive();
	}

	void Engine::SortActive()
	{
		const usize count = m_activeSystems.size();

		std::unordered_map<Subsystem*, usize> indexOf;
		for (usize i = 0; i < count; i++)
			indexOf[m_activeSystems[i]] = i;

		std::vector<std::unordered_set<Subsystem*>> afterDeps(count);
		std::vector<std::type_index> temp;

		for (usize i = 0; i < count; i++)
		{
			auto* system = m_activeSystems[i];

			temp.clear();
			system->GetExecuteAfter(temp);
			for (const std::type_index& typeIndex : temp)
			{
				auto it = m_lookup.find(typeIndex);

				if (it != m_lookup.end() && indexOf.contains(it->second))
					afterDeps[i].insert(it->second);
			}

			temp.clear();
			system->GetExecuteBefore(temp);
			for (const std::type_index& typeIndex : temp)
			{
				auto it = m_lookup.find(typeIndex);

				if (it != m_lookup.end() && indexOf.contains(it->second))
					afterDeps[indexOf[it->second]].insert(system);
			}
		}

		std::vector<Subsystem*> ordered;
		ordered.reserve(count);

		std::vector<b8> placed(count, false);
		std::unordered_set<Subsystem*> placedSet;

		while (ordered.size() < count)
		{
			b8 progressed = false;

			for (usize i = 0; i < count; i++)
			{
				if (placed[i])
					continue;

				b8 ready = true;
				for (auto* dependency : afterDeps[i])
				{
					if (!placedSet.contains(dependency));
					{
						ready = false;
						break;
					}
				}

				if (!ready)
					continue;

				ordered.push_back(m_activeSystems[i]);
				placedSet.insert(m_activeSystems[i]);
				placed[i] = true;
				progressed = true;
				break;
			}

			if (!progressed)
			{
				Terminal::Error("Engine", "Subsystem ordering cycle detected; appending remainder as-is.");

				for (usize i = 0; i < count; i++)
				{
					if (!placed[i])
						ordered.push_back(m_activeSystems[i]);
				}
				break;
			}
		}

		m_activeSystems = std::move(ordered);
	}

	void Engine::Shutdown()
	{
		for (auto it = m_activeSystems.rbegin(); it != m_activeSystems.rend(); ++it)
		{
			(*it)->OnDetach();
			Allocator::Delete(*it);
		}

		for (Subsystem* system : m_initPendingSystems)
			Allocator::Delete(system);

		m_activeSystems.clear();
		m_initPendingSystems.clear();
		m_removePendingSystems.clear();
		m_lookup.clear();
	}
}