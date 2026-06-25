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

			if (m_exitRequested)
				break;

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
		b8 changed = !m_removePendingSystems.empty() || !m_initPendingSystems.empty();

		for (Subsystem* system : m_removePendingSystems)
		{
			system->OnDetach();

			std::erase(m_activeSystems, system);

			for (auto it = m_lookup.begin(); it != m_lookup.end(); ++it)
			{
				if (it->second == system)
				{
					m_lookup.erase(it);
					break;
				}
			}

			Allocator::Delete(system);
		}
		m_removePendingSystems.clear();

		std::unordered_set<Subsystem*> attaching(m_initPendingSystems.begin(), m_initPendingSystems.end());

		for (Subsystem* system : m_initPendingSystems)
			m_activeSystems.push_back(system);
		m_initPendingSystems.clear();

		if (!changed)
			return;

		std::vector<Subsystem*> initOrder = BuildOrder(m_activeSystems, true);

		if (!m_exitRequested)
		{
			Terminal::Log("Engine", "### Subsystem Initialize Order ###");
			for (usize i = 0; i < initOrder.size(); i++)
				Terminal::Log("Engine", "Index ({}) - {}", i, initOrder[i]->GetName());

			for (Subsystem* system : initOrder)
			{
				if (attaching.contains(system))
				{
					EngineReport report = system->OnAttach(this);
					if (report)
					{
						Terminal::Error("Engine", "{} failed to attach: {}", system->GetName(), report.GetMessage());

						RequestExit("Subsystem attach failed.");
						return;
					}
				}
			}
		}

		SortActive();
	}

	void Engine::SortActive()
	{
		m_activeSystems = BuildOrder(m_activeSystems, false);

		if (m_exitRequested)
			return;

		Terminal::Log("Engine", "### Subsystem Execution Order ###");
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

	std::vector<Subsystem*> Engine::BuildOrder(const std::vector<Subsystem*>& systems, b8 initialize)
	{
		const usize count = systems.size();

		std::vector<OrderRules> rules(count);
		for (usize i = 0; i < count; i++)
		{
			if (initialize)
				systems[i]->GetInitializeOrder(rules[i]);
			else
				systems[i]->GetExecutionOrder(rules[i]);
		}

		std::unordered_map<Subsystem*, usize> indexOf;
		for (usize i = 0; i < count; i++)
			indexOf[systems[i]] = i;

		std::vector<std::unordered_set<Subsystem*>> afterDeps(count);

		for (usize a = 0; a < count; a++)
		{
			for (usize b = 0; b < count; b++)
			{
				if (a == b)
					continue;

				if (rules[a].tier < rules[b].tier)
					afterDeps[b].insert(systems[a]);
			}
		}

		for (usize i = 0; i < count; i++)
		{
			Subsystem* system = systems[i];

			for (const std::type_index& a : rules[i].after)
			{
				for (const std::type_index& b : rules[i].before)
				{
					if (a == b)
					{
						Terminal::Error("Engine", "{} declares After and Before on the same type {} ({} order).",
							system->GetName(), a.name(), initialize ? "Initialize" : "Execution");

						RequestExit("Subsystem ordering contradiction.");
						return systems;
					}
				}
			}

			for (const std::type_index& t : rules[i].after)
			{
				auto it = m_lookup.find(t);
				if (it != m_lookup.end() && indexOf.contains(it->second))
					afterDeps[i].insert(it->second);
			}

			for (const std::type_index& t : rules[i].before)
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

				ordered.push_back(systems[i]);
				placedSet.insert(systems[i]);
				placed[i] = true;
				progressed = true;
				break;
			}

			if (!progressed)
			{
				for (usize i = 0; i < count; i++)
				{
					if (!placed[i])
						Terminal::Error("Engine", "{} is stuck in a {} ordering cycle or tier conflict.",
							systems[i]->GetName(), initialize ? "Initialize" : "Execution");
				}

				RequestExit("Subsystem ordering could not be resolved.");
				return systems;
			}
		}

		return ordered;
	}

}