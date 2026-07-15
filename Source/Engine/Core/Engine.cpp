#include "Engine.h"

#include <Engine/Module/ModuleContext.h>

namespace Horizon
{
	Engine::Engine()
	{
		m_reflectionContext = Allocator::Create<ModuleContext>(CurrLoc());
		EngineReport report = m_reflectionContext->OnAttach(this);
		if (report)
		{
			Terminal::Error("Engine", "{} attach failed: {}", m_reflectionContext->GetName(), report.GetMessage());

			for (auto* pending : m_initPending)
				Allocator::Delete(pending);

			m_exitRequested = true;
		}
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

			for (System* system : m_activeSystems)
				system->OnSync();
		}

		Shutdown();

		m_reflectionContext->OnDetach();
		Allocator::Delete(m_reflectionContext);

		Allocator::ReportLeaks();
	}

	void Engine::RequestExit(std::string_view reason)
	{
		m_exitRequested = true;
		Terminal::Info("Engine", "Quit Reason - {}", reason);
	}

	void Engine::FlushPending()
	{
		for (EngineModule* pModule : m_removePending)
		{
			pModule->OnDetach();

			std::erase_if(m_activeSystems, [pModule](System* s) { return static_cast<EngineModule*>(s) == pModule; });
			std::erase_if(m_activeContexts, [pModule](Context* c) { return static_cast<EngineModule*>(c) == pModule; });
			std::erase(m_initOrder, pModule);
			m_lookup.erase(pModule->GetTypeId());

			Allocator::Delete(pModule);
		}
		m_removePending.clear();

		if (m_initPending.empty())
			return;

		std::vector<EngineModule*> added = m_initPending;

		for (EngineModule* pModule : m_initPending)
		{
			pModule->m_engine = this;
			m_lookup[pModule->GetTypeId()] = pModule;
			m_initOrder.push_back(pModule);

			if (auto* pSystem = dynamic_cast<System*>(pModule))
				m_activeSystems.push_back(pSystem);
			else if (auto* pContext = dynamic_cast<Context*>(pModule))
				m_activeContexts.push_back(pContext);
		}
		m_initPending.clear();

		SortActive();

		for (EngineModule* pModule : m_initOrder)
		{
			if (std::find(added.begin(), added.end(), pModule) == added.end())
				continue;

			EngineReport report = pModule->OnAttach(this);
			if (report)
				Terminal::Warn("Engine", "{} attach failed: {}", pModule->GetName(), report.GetMessage());
		}
	}

	void Engine::SortActive()
	{
		if (m_exitRequested)
			return;

		m_initOrder = Build(m_initOrder,
			[](EngineModule* pModule, OrderRules& rules) { pModule->GetInitializeOrder(rules); });

		std::vector<EngineModule*> systems(m_activeSystems.begin(), m_activeSystems.end());
		systems = Build(systems, [](EngineModule* pModule, OrderRules& rules) { static_cast<System*>(pModule)->GetExecutionOrder(rules); });

		m_activeSystems.clear();
		m_activeSystems.reserve(systems.size());

		for (EngineModule* pModule : systems)
			m_activeSystems.push_back(static_cast<System*>(pModule));
	}

	void Engine::Shutdown()
	{
		for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
			(*it)->OnDetach();

		for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
			Allocator::Delete(*it);

		m_activeSystems.clear();
		m_activeContexts.clear();
		m_initOrder.clear();
		m_lookup.clear();
	}

	std::vector<EngineModule*> Engine::Build(const std::vector<EngineModule*>& modules, void(*getRules)(EngineModule*, OrderRules&)) const
	{
		usize n = modules.size();

		std::unordered_map<std::type_index, usize> indexOf;
		for (usize i = 0; i < n; ++i)
			indexOf[modules[i]->GetTypeId()] = i;

		std::vector<std::vector<usize>> adjacency(n);
		std::vector<u32> indegree(n, 0);
		std::vector<OrderTier> tiers(n, OrderTier::Default);

		for (usize i = 0; i < n; ++i)
		{
			OrderRules rules;
			getRules(modules[i], rules);
			tiers[i] = rules.tier;

			for (const std::type_index& dep : rules.after)
			{
				auto it = indexOf.find(dep);
				if (it != indexOf.end())
				{
					adjacency[it->second].push_back(i);
					indegree[i]++;
				}
			}

			for (const std::type_index& other : rules.before)
			{
				auto it = indexOf.find(other);
				if (it != indexOf.end())
				{
					adjacency[i].push_back(it->second);
					indegree[it->second]++;
				}
			}
		}

		std::vector<usize> ready;
		for (usize i = 0; i < n; ++i)
		{
			if (indegree[i] == 0)
				ready.push_back(i);
		}

		std::vector<EngineModule*> ordered;
		ordered.reserve(n);

		while (!ready.empty())
		{
			usize best = 0;
			for (usize k = 1; k < ready.size(); ++k)
			{
				if (tiers[ready[k]] < tiers[ready[best]])
					best = k;
			}

			usize node = ready[best];
			ready[best] = ready.back();
			ready.pop_back();

			ordered.push_back(modules[node]);

			for (usize next : adjacency[node])
			{
				if (--indegree[next] == 0)
					ready.push_back(next);
			}
		}

		if (ordered.size() != n)
			Terminal::Warn("Engine", "Cyclic module dependency; only {} of {} ordered", ordered.size(), n);

		return ordered;
	}
}