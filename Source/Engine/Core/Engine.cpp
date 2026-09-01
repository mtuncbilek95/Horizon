#include "Engine.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Job/JobSystem.h>

#include <unordered_set>

namespace Horizon::Engine
{
	Engine::Engine()
	{
		m_reflectionSystem = Memory::Allocator::Create<ReflectionSystem>(Memory::CurrLoc(), this);
		m_jobSystem = Memory::Allocator::Create<JobSystem>(Memory::CurrLoc());
	}

	Engine::~Engine()
	{
		Memory::Allocator::Delete(m_jobSystem);
		Memory::Allocator::Delete(m_reflectionSystem);
		Memory::Allocator::ReportLeaks();
	}

	void Engine::Run()
	{
		m_timer.Start();

		while (m_running)
		{
			FlushPending();

			f64 deltaTime = m_timer.GetElapsedTimeInSec();
			m_timer.Reset();

			m_frameContext.Advance(deltaTime);

			for (Service* pService : m_activeServices)
				pService->OnExecute(m_frameContext);
		}

		Shutdown();
	}

	void Engine::RequestExit(std::string_view reason)
	{
		if (!m_running)
			return;

		Terminal::Info(StringOps::GetName(this), "Exit requested: {}", reason);
		m_running = false;
	}

	b8 Engine::IsRegistered(std::type_index index) const
	{
		if (m_lookupServices.contains(index) || m_lookupContexts.contains(index))
			return true;

		for (Module* pModule : m_registerPending)
		{
			if (pModule->GetTypeId() == index)
				return true;
		}

		return false;
	}

	void Engine::FlushPending()
	{
		while (!m_registerPending.IsEmpty())
		{
			List<Module*> batch;
			batch.Swap(m_registerPending);

			std::unordered_map<std::type_index, Module*> batchById;
			List<std::type_index> nodes;
			nodes.Reserve(batch.GetCount());

			m_dependencyGraph.Clear();

			for (Module* pModule : batch)
			{
				const std::type_index index = pModule->GetTypeId();

				batchById.emplace(index, pModule);
				nodes.PushBack(index);

				m_dependencyGraph.BeginNode(index);
				pModule->DeclareDependencies(m_dependencyGraph);
			}

			std::unordered_set<std::type_index> satisfied;

			for (const auto& pair : m_lookupServices)
				satisfied.insert(pair.first);

			for (const auto& pair : m_lookupContexts)
				satisfied.insert(pair.first);

			List<std::type_index> order;

			if (!m_dependencyGraph.Resolve(nodes, satisfied, order))
			{
				for (Module* pModule : batch)
					Memory::Allocator::Delete(pModule);

				RequestExit("Module dependency graph could not be resolved");
				return;
			}

			for (const auto& index : order)
				Activate(batchById[index]);
		}
	}

	void Engine::Activate(Module* pModule)
	{
		const std::type_index index = pModule->GetTypeId();
		ModuleReport report = pModule->OnInitialize();

		if (report)
		{
			Terminal::Fatal(StringOps::GetName(this), "{} failed to initialize: {}", StringOps::GetNameString(index.name()), report.GetMessage());
			Memory::Allocator::Delete(pModule);
			RequestExit("Module initialization failed");
			return;
		}

		if (Service* pService = dynamic_cast<Service*>(pModule))
		{
			m_lookupServices.emplace(index, m_activeServices.GetCount());
			m_activeServices.PushBack(pService);
		}
		else
		{
			m_lookupContexts.emplace(index, m_activeContexts.GetCount());
			m_activeContexts.PushBack(static_cast<Context*>(pModule));
		}

		m_shutdownOrder.PushBack(pModule);
		Terminal::Info(StringOps::GetName(this), "{} is active", StringOps::GetNameString(index.name()));
	}

	void Engine::Shutdown()
	{
		for (usize i = m_shutdownOrder.GetCount(); i > 0; --i)
			m_shutdownOrder[i - 1]->OnFinalize();

		for (usize i = m_shutdownOrder.GetCount(); i > 0; --i)
			Memory::Allocator::Delete(m_shutdownOrder[i - 1]);

		for (Module* pModule : m_registerPending)
			Memory::Allocator::Delete(pModule);

		m_shutdownOrder.Clear();
		m_registerPending.Clear();
		m_activeServices.Clear();
		m_activeContexts.Clear();
		m_lookupServices.clear();
		m_lookupContexts.clear();
		m_dependencyGraph.Clear();
	}
}