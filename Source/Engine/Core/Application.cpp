#include "Application.h"

#include <Engine/Module/ModuleContext.h>

#include <algorithm>

namespace Horizon::Engine
{
	Application::Application()
	{
		m_hostLibrary = Memory::Allocator::Create<PAL::SymbolLibrary>(Memory::CurrLoc(), PAL::SymbolLibraryDesc());
		if (!m_hostLibrary)
		{
			Terminal::Error("Application", "Host Library could not been created");
			m_exitRequested = true;
			return;
		}

		m_reflectionContext = Memory::Allocator::Create<ModuleContext>(Memory::CurrLoc(), m_hostLibrary);
		AppReport report = m_reflectionContext->OnAttach(this);
		if (report)
		{
			Terminal::Error("Application", "{} attach failed: {}", m_reflectionContext->GetName(), report.GetMessage());

			for (auto* pending : m_initPending)
				Memory::Allocator::Delete(pending);

			m_exitRequested = true;
		}
	}

	Application::~Application()
	{
	}

	void Application::Run()
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
		Memory::Allocator::Delete(m_reflectionContext);
		Memory::Allocator::Delete(m_hostLibrary);

		Memory::Allocator::ReportLeaks();
	}

	void Application::RequestExit(std::string_view reason)
	{
		m_exitRequested = true;
		Terminal::Info("Application", "Quit Reason - {}", reason);
	}

	void Application::FlushPending()
	{
		for (AppModule* pModule : m_removePending)
		{
			pModule->OnDetach();

			for (i64 i = (i64)m_activeSystems.GetCount() - 1; i >= 0; --i)
			{
				if (static_cast<AppModule*>(m_activeSystems[(usize)i]) == pModule)
					m_activeSystems.RemoveAt((usize)i);
			}

			for (i64 i = (i64)m_activeContexts.GetCount() - 1; i >= 0; --i)
			{
				if (static_cast<AppModule*>(m_activeContexts[(usize)i]) == pModule)
					m_activeContexts.RemoveAt((usize)i);
			}

			m_initOrder.Remove(pModule);
			m_lookup.erase(pModule->GetTypeId());

			Memory::Allocator::Delete(pModule);
		}
		m_removePending.Clear();

		if (m_initPending.IsEmpty())
			return;

		List<AppModule*> added = m_initPending;

		for (AppModule* pModule : m_initPending)
		{
			pModule->m_engine = this;
			m_lookup[pModule->GetTypeId()] = pModule;
			m_initOrder.PushBack(pModule);

			if (auto* pSystem = dynamic_cast<System*>(pModule))
				m_activeSystems.PushBack(pSystem);
			else if (auto* pContext = dynamic_cast<Context*>(pModule))
				m_activeContexts.PushBack(pContext);
		}
		m_initPending.Clear();

		SortActive();

		for (AppModule* pModule : m_initOrder)
		{
			if (std::find(added.begin(), added.end(), pModule) == added.end())
				continue;

			AppReport report = pModule->OnAttach(this);
			if (report)
				Terminal::Warn("Application", "{} attach failed: {}", pModule->GetName(), report.GetMessage());
		}
	}

	void Application::SortActive()
	{
		if (m_exitRequested)
			return;

		m_initOrder = Build(m_initOrder,
			[](AppModule* pModule, OrderRules& rules) { pModule->GetInitializeOrder(rules); });

		List<AppModule*> systems;
		systems.Reserve(m_activeSystems.GetCount());
		for (System* pSystem : m_activeSystems)
			systems.PushBack(pSystem);

		systems = Build(systems, [](AppModule* pModule, OrderRules& rules) { static_cast<System*>(pModule)->GetExecutionOrder(rules); });

		m_activeSystems.Clear();
		m_activeSystems.Reserve(systems.GetCount());

		for (AppModule* pModule : systems)
			m_activeSystems.PushBack(static_cast<System*>(pModule));
	}

	void Application::Shutdown()
	{
		for(auto it = m_initPending.rbegin(); it != m_initPending.rend(); ++it)
			Memory::Allocator::Delete(*it);

		for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
			(*it)->OnDetach();

		for (auto it = m_initOrder.rbegin(); it != m_initOrder.rend(); ++it)
			Memory::Allocator::Delete(*it);

		m_activeSystems.Clear();
		m_activeContexts.Clear();
		m_initOrder.Clear();
		m_lookup.clear();
	}

	List<AppModule*> Application::Build(const List<AppModule*>& modules, void(*getRules)(AppModule*, OrderRules&)) const
	{
		usize n = modules.GetCount();

		std::unordered_map<std::type_index, usize> indexOf;
		for (usize i = 0; i < n; ++i)
			indexOf[modules[i]->GetTypeId()] = i;

		List<List<usize>> adjacency(n);
		List<u32> indegree(n, 0);
		List<OrderTier> tiers(n, OrderTier::Default);

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
					adjacency[it->second].PushBack(i);
					indegree[i]++;
				}
			}

			for (const std::type_index& other : rules.before)
			{
				auto it = indexOf.find(other);
				if (it != indexOf.end())
				{
					adjacency[i].PushBack(it->second);
					indegree[it->second]++;
				}
			}
		}

		List<usize> ready;
		for (usize i = 0; i < n; ++i)
		{
			if (indegree[i] == 0)
				ready.PushBack(i);
		}

		List<AppModule*> ordered;
		ordered.Reserve(n);

		while (!ready.IsEmpty())
		{
			usize best = 0;
			for (usize k = 1; k < ready.GetCount(); ++k)
			{
				if (tiers[ready[k]] < tiers[ready[best]])
					best = k;
			}

			usize node = ready[best];
			ready[best] = ready.Back();
			ready.PopBack();

			ordered.PushBack(modules[node]);

			for (usize next : adjacency[node])
			{
				if (--indegree[next] == 0)
					ready.PushBack(next);
			}
		}

		if (ordered.GetCount() != n)
			Terminal::Warn("Application", "Cyclic module dependency; only {} of {} ordered", ordered.GetCount(), n);

		return ordered;
	}
}