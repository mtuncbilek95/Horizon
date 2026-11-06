#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Core/Service.h>
#include <Engine/Core/ModuleDependency.h>
#include <Engine/Core/EngineFrame.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Timer/Timer.h>

#include <typeindex>
#include <unordered_map>

namespace Horizon::Engine
{
	class Engine final
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
				Terminal::Warn(StringHelper::GetTypeName(this), "{} is already registered", index.name());
				return;
			}
			T* pModule = new T(std::forward<Args>(args)...);
			if (!pModule)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} could not be allocated", index.name());
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
				Terminal::Error(StringHelper::GetTypeName(this), "{} could not found", typeid(T).name());
				return nullptr;
			}
			return (T*)m_activeServices[it->second].Get();
		}

		template<typename T>
			requires std::is_base_of_v<Context, T>
		T* RequestContext()
		{
			auto it = m_lookupContexts.find(typeid(T));
			if (it == m_lookupContexts.end())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} could not found", typeid(T).name());
				return nullptr;
			}
			return (T*)m_activeContexts[it->second].Get();
		}

		void Run();
		void RequestExit(std::string_view reason);

		ReflectionSystem* GetReflectionSystem() const noexcept { return m_reflectionSystem.Get(); }
		JobSystem* GetJobSystem() const noexcept { return m_jobSystem.Get(); }
		const EngineFrame& GetFrame() const { return m_frameContext; }

	private:
		b8 IsRegistered(std::type_index index) const;
		b8 IsActive(std::type_index index) const;
		void FlushPending();
		b8 ResolveInitOrder(const List<Module*>& batch, List<usize>& outOrder);
		b8 SortServices();
		void Activate(Module* pModule);
		void Shutdown();

	private:
		PAL::Timer m_timer;
		EngineFrame m_frameContext;

		List<Module*> m_registerPending;
		List<Module*> m_shutdownOrder;

		List<UniquePtr<Service>> m_activeServices;
		std::unordered_map<std::type_index, usize> m_lookupServices;

		List<UniquePtr<Context>> m_activeContexts;
		std::unordered_map<std::type_index, usize> m_lookupContexts;

		UniquePtr<ReflectionSystem> m_reflectionSystem;
		UniquePtr<JobSystem> m_jobSystem;

		b8 m_running = true;
	};
}