#pragma once

#include <Engine/Core/EngineModule.h>
#include <Engine/Core/System.h>
#include <Engine/Core/Context.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>

#include <string_view>
#include <vector>
#include <unordered_map>
#include <typeindex>

namespace Horizon
{
	class ModuleContext;

	class H_EXPORT Engine final
	{
	public:
		Engine();
		~Engine();

		template<typename TSystem, typename... Args>
			requires std::is_base_of_v<System, TSystem>
		TSystem& AddSystem(Args&&... args)
		{
			std::type_index key = std::type_index(typeid(TSystem));

			auto* pSystem = Allocator::Create<TSystem>(CurrLoc(), std::forward<Args>(args)...);
			m_initPending.push_back(pSystem);

			return *pSystem;
		}

		template<typename TSystem>
			requires std::is_base_of_v<System, TSystem>
		void RemoveSystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TSystem)));
			if (it == m_lookup.end())
			{
				Terminal::Warn("Engine", "{} is not registered.", typeid(TSystem).name());
				return;
			}

			m_removePending.push_back(it->second);
		}

		template<typename TContext, typename... Args>
			requires std::is_base_of_v<Context, TContext>
		TContext& AddContext(Args&&... args)
		{
			std::type_index key = std::type_index(typeid(TContext));

			auto* context = Allocator::Create<TContext>(CurrLoc(), std::forward<Args>(args)...);
			m_initPending.push_back(context);
			return *context;
		}

		template<typename TContext, typename... Args>
			requires std::is_base_of_v<Context, TContext>
		void RemoveContext()
		{
			auto it = m_lookup.find(std::type_index(typeid(TContext)));
			if (it == m_lookup.end())
			{
				Terminal::Warn("Engine", "{} is not registered.", typeid(TContext).name());
				return;
			}

			m_removePending.push_back(it->second);
		}

		template<typename TSystem>
			requires std::is_base_of_v<System, TSystem>
		TSystem& GetSystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TSystem)));
			Terminal::Assert(it != m_lookup.end(), "Engine", "System not found");

			return *static_cast<TSystem*>(it->second);
		}

		template<typename TSystem>
			requires std::is_base_of_v<System, TSystem>
		TSystem* TryGetSystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TSystem)));
			return it == m_lookup.end() ? nullptr : static_cast<TSystem*>(it->second);
		}

		template<typename TContext>
			requires std::is_base_of_v<Context, TContext>
		TContext& GetContext()
		{
			auto it = m_lookup.find(std::type_index(typeid(TContext)));
			Terminal::Assert(it != m_lookup.end(), "Engine", "System not found");

			return *static_cast<TContext*>(it->second);
		}

		template<typename TContext>
			requires std::is_base_of_v<Context, TContext>
		TContext* TryGetContext()
		{
			auto it = m_lookup.find(std::type_index(typeid(TContext)));
			return it == m_lookup.end() ? nullptr : static_cast<TContext*>(it->second);
		}

		void Run();
		void RequestExit(std::string_view reason);

		ModuleContext* GetModuleContext() const { return m_reflectionContext; }

	private:
		void FlushPending();
		void SortActive();
		void Shutdown();

		std::vector<EngineModule*> Build(const std::vector<EngineModule*>& modules,
			void(*getRules)(EngineModule*, OrderRules&)) const;

	private:
		ModuleContext* m_reflectionContext = nullptr;
		PAL::SymbolLibrary* m_hostLibrary = nullptr;

		std::vector<System*> m_activeSystems;
		std::vector<Context*> m_activeContexts;

		std::vector<EngineModule*> m_initOrder;
		std::vector<EngineModule*> m_initPending;
		std::vector<EngineModule*> m_removePending;

		std::unordered_map<std::type_index, EngineModule*> m_lookup;
		b8 m_exitRequested = false;
	};
}