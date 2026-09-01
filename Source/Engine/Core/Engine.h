#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Core/Service.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Core/EngineFrame.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Timer/Timer.h>

#include <typeindex>
#include <unordered_map>

namespace Horizon::Engine
{
	class ReflectionSystem;
	class JobSystem;

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
		JobSystem* GetJobSystem() const noexcept { return m_jobSystem; }
	private:
		b8 IsRegistered(std::type_index index) const;
		void FlushPending();
		void Activate(Module* pModule);
		void Shutdown();
	private:
		PAL::Timer m_timer;
		EngineFrame m_frameContext;

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