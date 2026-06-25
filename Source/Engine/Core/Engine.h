#pragma once

#include <Engine/Core/Subsystem.h>

#include <string_view>
#include <vector>
#include <unordered_map>
#include <typeindex>

namespace Horizon
{
	class Engine final
	{
	public:
		Engine();
		~Engine();

		template<typename TModule, typename... Args>
			requires std::is_base_of_v<Subsystem, TModule>
		TModule& AddModule(Args&&... args)
		{
			std::type_index key = std::type_index(typeid(TModule));

			auto it = m_lookup.find(key);
			if (it != m_lookup.end())
			{
				Terminal::Warn("Engine", "{} has already been registered.", key.name());
				return *static_cast<TModule*>(it->second);
			}

			auto* system = Allocator::Create<TModule>(CurrLoc(), std::forward<Args>(args)...);

			m_lookup.emplace(key, system);
			m_initPendingSystems.push_back(system);

			return *system;
		}

		template<typename TModule>
		void RemoveModule()
		{
			auto it = m_lookup.find(std::type_index(typeid(TModule)));
			if (it == m_lookup.end())
			{
				Terminal::Warn("Engine", "{} is not registered.", typeid(TModule).name());
				return;
			}

			m_removePendingSystems.push_back(it->second);
		}

		template<typename TModule>
		TModule& GetModule()
		{
			auto it = m_lookup.find(std::type_index(typeid(TModule)));
			Terminal::Assert(it != m_lookup.end(), "Engine", "Module not found");

			return *static_cast<TModule*>(it->second);
		}

		template<typename TModule>
		TModule* TryGetModule()
		{
			auto it = m_lookup.find(std::type_index(typeid(TModule)));
			return it == m_lookup.end() ? nullptr : static_cast<TModule*>(it->second);
		}

		void Run();
		void RequestExit(std::string_view reason);

	private:
		void FlushPending();
		void SortActive();
		void Shutdown();

	private:
		std::vector<Subsystem*> m_activeSystems;
		std::vector<Subsystem*> m_initPendingSystems;
		std::vector<Subsystem*> m_removePendingSystems;

		std::unordered_map<std::type_index, Subsystem*> m_lookup;
		b8 m_exitRequested = false;
	};
}