#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Core/Service.h>
#include <Engine/Core/ModuleGraph.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <typeindex>
#include <unordered_map>

namespace Horizon::Engine
{
	class ReflectionSystem;
	class JobSystem;

	/*
	* Engine owns every Context and Service instance. RegisterModule only
	* allocates and parks the module in the pending list; nothing is
	* initialized there, because a module cannot declare its dependencies
	* before the modules it depends on exist. Run flushes the pending list
	* at the top of every frame, so a Service may register new modules
	* while it executes without mutating the list being iterated.
	*/
	class H_EXPORT Engine final
	{
	public:
		Engine();
		~Engine();

		template<typename T, typename...Args>
			requires std::is_base_of_v<Context, T> || std::is_base_of_v<Service, T>
		void RegisterModule(Args&&... args)
		{
			const std::type_index index = std::type_index(typeid(T));

			if (IsRegistered(index))
			{
				Terminal::Warn(StringOps::GetName(this), "{} is already registered", index.name());
				return;
			}

			T* pModule = Memory::Allocator::Create<T>(Memory::CurrLoc(), std::forward<Args>(args)...);

			if (!pModule)
			{
				Terminal::Error(StringOps::GetName(this), "{} could not be allocated", index.name());
				return;
			}

			pModule->m_engine = this;
			m_registerPending.PushBack(pModule);
		}

		template<typename T>
			requires std::is_base_of_v<Service, T>
		T* RequestService()
		{
			auto it = m_lookupServices.find(typeid(T));
			if (it == m_lookupServices.end())
			{
				Terminal::Error(StringOps::GetName(this), "{} could not found", typeid(T).name());
				return nullptr;
			}

			return (T*)m_activeServices[it->second];
		}

		template<typename T>
			requires std::is_base_of_v<Context, T>
		T* RequestContext()
		{
			auto it = m_lookupContexts.find(typeid(T));
			if (it == m_lookupContexts.end())
			{
				Terminal::Error(StringOps::GetName(this), "{} could not found", typeid(T).name());
				return nullptr;
			}

			return (T*)m_activeContexts[it->second];
		}

		void Run();
		void RequestExit(std::string_view reason);

		ReflectionSystem* GetReflectionSystem() const noexcept { return m_reflectionSystem; }

	private:
		b8 IsRegistered(std::type_index index) const;
		void FlushPending();
		void Activate(Module* pModule);
		void Shutdown();

	private:
		ModuleGraph m_dependencyGraph;
		List<Module*> m_registerPending;
		List<Module*> m_shutdownOrder;

		List<Service*> m_activeServices;
		std::unordered_map<std::type_index, usize> m_lookupServices;

		List<Context*> m_activeContexts;
		std::unordered_map<std::type_index, usize> m_lookupContexts;

		ReflectionSystem* m_reflectionSystem = nullptr;
		JobSystem* m_jobSystem = nullptr;

		b8 m_running = true;
	};
}