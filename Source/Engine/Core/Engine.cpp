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

		std::unordered_set<Subsystem*> attaching(m_initPendingSystems.begin(), m_initPendingSystems.end());

		for (auto* system : m_initPendingSystems)
			m_activeSystems.push_back(system);

		m_initPendingSystems.clear();

		if (changed)
			SortActive();

		if (!m_exitRequested)
		{
			for (Subsystem* system : m_activeSystems)
			{
				if (attaching.contains(system))
					system->OnAttach(this);
			}
		}
	}

	void Engine::SortActive()
	{
		const usize count = m_activeSystems.size();

		std::unordered_map<Subsystem*, usize> indexOf;
		for (usize i = 0; i < count; i++)
			indexOf[m_activeSystems[i]] = i;

		std::vector<std::unordered_set<Subsystem*>> afterDeps(count);

		for (usize a = 0; a < count; a++)
		{
			for (usize b = 0; b < count; b++)
			{
				if (a == b)
					continue;

				if (m_activeSystems[a]->GetExecutionTier() < m_activeSystems[b]->GetExecutionTier())
					afterDeps[b].insert(m_activeSystems[a]);
			}
		}

		std::vector<std::type_index> afterList;
		std::vector<std::type_index> beforeList;

		for (usize i = 0; i < count; i++)
		{
			Subsystem* system = m_activeSystems[i];

			afterList.clear();
			beforeList.clear();
			system->GetExecuteAfter(afterList);
			system->GetExecuteBefore(beforeList);

			for (const std::type_index& a : afterList)
			{
				for (const std::type_index& b : beforeList)
				{
					if (a == b)
					{
						Terminal::Error("Engine", "{} declares ExecuteAfter and ExecuteBefore on the same type {}.",
							typeid(*system).name(), a.name());

						RequestExit("Subsystem ordering contradiction.");
						return;
					}
				}
			}

			for (const std::type_index& t : afterList)
			{
				auto it = m_lookup.find(t);
				if (it != m_lookup.end() && indexOf.contains(it->second))
					afterDeps[i].insert(it->second);
			}

			for (const std::type_index& t : beforeList)
			{
				auto it = m_lookup.find(t);
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
				for (Subsystem* dep : afterDeps[i])
				{
					if (!placedSet.contains(dep))
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
				for (usize i = 0; i < count; i++)
				{
					if (!placed[i])
						Terminal::Error("Engine", "{} is stuck in an ordering cycle or tier conflict.", typeid(*m_activeSystems[i]).name());
				}

				RequestExit("Subsystem ordering could not be resolved.");
				return;
			}
		}

		m_activeSystems = std::move(ordered);

		Terminal::Log("Engine", "### Subsystem Order List ###");
		for (usize i = 0; i < m_activeSystems.size(); i++)
			Terminal::Log("Engine", "Index ({}) - {}", i, m_activeSystems[i]->GetName());

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