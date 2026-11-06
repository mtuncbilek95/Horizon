#include "Engine.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Job/JobSystem.h>

namespace Horizon::Engine
{
	namespace
	{
		b8 StableTopologicalSort(const List<List<usize>>& adjacency, List<usize>& outOrder)
		{
			const usize count = adjacency.GetCount();

			List<usize> indegree(count);
			for (const List<usize>& edges : adjacency)
			{
				for (usize after : edges)
					indegree[after]++;
			}

			List<b8> emitted(count);

			outOrder.Clear();
			outOrder.Reserve(count);

			while (outOrder.GetCount() < count)
			{
				usize next = count;

				for (usize i = 0; i < count; i++)
				{
					if (!emitted[i] && indegree[i] == 0)
					{
						next = i;
						break;
					}
				}

				if (next == count)
					return false;

				emitted[next] = true;
				outOrder.PushBack(next);

				for (usize after : adjacency[next])
					indegree[after]--;
			}

			return true;
		}
	}

	Engine::Engine()
	{
		m_reflectionSystem = MakeUnique<ReflectionSystem>(this);
		m_jobSystem = MakeUnique<JobSystem>();
	}

	Engine::~Engine()
	{
		m_jobSystem = nullptr;
		m_reflectionSystem = nullptr;
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

			for (const UniquePtr<Service>& pService : m_activeServices)
				pService->OnExecute(m_frameContext);
		}

		Shutdown();
	}

	void Engine::RequestExit(std::string_view reason)
	{
		if (!m_running)
			return;

		Terminal::Info(StringHelper::GetTypeName(this), "Exit requested: {}", reason);
		m_running = false;
	}

	b8 Engine::IsRegistered(std::type_index index) const
	{
		if (IsActive(index))
			return true;

		for (Module* pModule : m_registerPending)
		{
			if (pModule->GetTypeId() == index)
				return true;
		}

		return false;
	}

	b8 Engine::IsActive(std::type_index index) const
	{
		return m_lookupServices.contains(index) || m_lookupContexts.contains(index);
	}

	void Engine::FlushPending()
	{
		while (!m_registerPending.IsEmpty())
		{
			List<Module*> batch;
			batch.Swap(m_registerPending);

			for (Module* pModule : batch)
				pModule->DeclareDependencies(pModule->m_dependency);

			List<usize> order;

			if (!ResolveInitOrder(batch, order))
			{
				for (Module* pModule : batch)
					delete pModule;

				RequestExit("Module dependency graph could not be resolved");
				return;
			}

			const usize serviceCount = m_activeServices.GetCount();

			for (usize index : order)
				Activate(batch[index]);

			if (m_activeServices.GetCount() == serviceCount)
				continue;

			if (!SortServices())
			{
				RequestExit("Service run order could not be resolved");
				return;
			}
		}
	}

	b8 Engine::ResolveInitOrder(const List<Module*>& batch, List<usize>& outOrder)
	{
		std::unordered_map<std::type_index, usize> batchLookup;

		for (usize i = 0; i < batch.GetCount(); i++)
			batchLookup.emplace(batch[i]->GetTypeId(), i);

		List<List<usize>> adjacency(batch.GetCount());

		for (usize i = 0; i < batch.GetCount(); i++)
		{
			const std::type_index owner = batch[i]->GetTypeId();
			const ModuleDependency& dependency = batch[i]->m_dependency;

			for (const std::type_index& target : dependency.m_initAfterList)
			{
				if (IsActive(target))
					continue;

				auto it = batchLookup.find(target);
				if (it == batchLookup.end())
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "{} inits after unregistered module {}, dropping the constraint", StringHelper::GetNameString(owner.name()), StringHelper::GetNameString(target.name()));
					continue;
				}

				adjacency[it->second].PushBack(i);
			}

			for (const std::type_index& target : dependency.m_initBeforeList)
			{
				if (IsActive(target))
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "{} inits before {} but {} is already active, dropping the constraint", StringHelper::GetNameString(owner.name()), StringHelper::GetNameString(target.name()), StringHelper::GetNameString(target.name()));
					continue;
				}

				auto it = batchLookup.find(target);
				if (it == batchLookup.end())
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "{} inits before unregistered module {}, dropping the constraint", StringHelper::GetNameString(owner.name()), StringHelper::GetNameString(target.name()));
					continue;
				}

				adjacency[i].PushBack(it->second);
			}

			const b8 hasRunConstraints = !dependency.m_runBeforeList.IsEmpty() || !dependency.m_runAfterList.IsEmpty();

			if (hasRunConstraints && !dynamic_cast<Service*>(batch[i]))
				Terminal::Warn(StringHelper::GetTypeName(this), "{} is a context, its run constraints are dropped", StringHelper::GetNameString(owner.name()));
		}

		if (!StableTopologicalSort(adjacency, outOrder))
		{
			Terminal::Fatal(StringHelper::GetTypeName(this), "Cyclic init dependency detected inside the pending module batch");
			return false;
		}

		return true;
	}

	b8 Engine::SortServices()
	{
		const usize count = m_activeServices.GetCount();

		List<List<usize>> adjacency(count);

		for (usize i = 0; i < count; i++)
		{
			const std::type_index owner = m_activeServices[i]->GetTypeId();
			const ModuleDependency& dependency = m_activeServices[i]->m_dependency;

			for (const std::type_index& target : dependency.m_runBeforeList)
			{
				auto it = m_lookupServices.find(target);
				if (it == m_lookupServices.end())
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "{} runs before {} which is not an active service, dropping the constraint", StringHelper::GetNameString(owner.name()), StringHelper::GetNameString(target.name()));
					continue;
				}

				adjacency[i].PushBack(it->second);
			}

			for (const std::type_index& target : dependency.m_runAfterList)
			{
				auto it = m_lookupServices.find(target);
				if (it == m_lookupServices.end())
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "{} runs after {} which is not an active service, dropping the constraint", StringHelper::GetNameString(owner.name()), StringHelper::GetNameString(target.name()));
					continue;
				}

				adjacency[it->second].PushBack(i);
			}
		}

		List<usize> order;

		if (!StableTopologicalSort(adjacency, order))
		{
			Terminal::Fatal(StringHelper::GetTypeName(this), "Cyclic run dependency detected between active services");
			return false;
		}

		List<UniquePtr<Service>> sorted;
		sorted.Reserve(count);

		for (usize index : order)
			sorted.PushBack(std::move(m_activeServices[index]));

		m_activeServices.Swap(sorted);
		m_lookupServices.clear();

		for (usize i = 0; i < count; i++)
			m_lookupServices.emplace(m_activeServices[i]->GetTypeId(), i);

		return true;
	}

	void Engine::Activate(Module* pModule)
	{
		const std::type_index index = pModule->GetTypeId();
		ModuleReport report = pModule->OnInitialize();

		if (report)
		{
			Terminal::Fatal(StringHelper::GetTypeName(this), "{} failed to initialize: {}", StringHelper::GetNameString(index.name()), report.GetMessage());
			delete pModule;
			RequestExit("Module initialization failed");
			return;
		}

		if (Service* pService = dynamic_cast<Service*>(pModule))
		{
			m_lookupServices.emplace(index, m_activeServices.GetCount());
			m_activeServices.PushBack(UniquePtr<Service>(pService));
		}
		else
		{
			m_lookupContexts.emplace(index, m_activeContexts.GetCount());
			m_activeContexts.PushBack(UniquePtr<Context>(static_cast<Context*>(pModule)));
		}

		m_shutdownOrder.PushBack(pModule);
		Terminal::Info(StringHelper::GetTypeName(this), "{} is active", StringHelper::GetNameString(index.name()));
	}

	void Engine::Shutdown()
	{
		for (usize i = m_shutdownOrder.GetCount(); i > 0; --i)
			m_shutdownOrder[i - 1]->OnFinalize();

		for (usize i = m_shutdownOrder.GetCount(); i > 0; --i)
		{
			const std::type_index index = m_shutdownOrder[i - 1]->GetTypeId();

			auto service = m_lookupServices.find(index);
			if (service != m_lookupServices.end())
			{
				m_activeServices[service->second].Reset();
				continue;
			}

			auto context = m_lookupContexts.find(index);
			if (context != m_lookupContexts.end())
				m_activeContexts[context->second].Reset();
		}

		for (Module* pModule : m_registerPending)
			delete pModule;

		m_shutdownOrder.Clear();
		m_registerPending.Clear();
		m_activeServices.Clear();
		m_activeContexts.Clear();
		m_lookupServices.clear();
		m_lookupContexts.clear();
	}
}