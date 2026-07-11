#pragma once

#include <Engine/Core/System.h>

#include <string_view>
#include <vector>
#include <unordered_map>
#include <typeindex>

namespace Horizon
{
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

			auto it = m_lookup.find(key);
			if (it != m_lookup.end())
			{
				Terminal::Warn("Engine", "{} has already been registered.", key.name());
				return *static_cast<TSystem*>(it->second);
			}

			auto* system = Allocator::Create<TSystem>(CurrLoc(), std::forward<Args>(args)...);

			m_lookup.emplace(key, system);
			m_initPendingSystems.push_back(system);

			return *system;
		}

		template<typename TSystem>
		void RemoveSystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TSystem)));
			if (it == m_lookup.end())
			{
				Terminal::Warn("Engine", "{} is not registered.", typeid(TSystem).name());
				return;
			}

			m_removePendingSystems.push_back(it->second);
		}

		template<typename TSystem>
		TSystem& GetSystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TSystem)));
			Terminal::Assert(it != m_lookup.end(), "Engine", "System not found");

			return *static_cast<TSystem*>(it->second);
		}

		template<typename TSystem>
		TSystem* TryGetSystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TSystem)));
			return it == m_lookup.end() ? nullptr : static_cast<TSystem*>(it->second);
		}

		void Run();
		void RequestExit(std::string_view reason);

	private:
		void FlushPending();
		void SortActive();
		void Shutdown();

		std::vector<System*> BuildOrder(const std::vector<System*>& systems, b8 initialize);

	private:
		std::vector<System*> m_activeSystems;
		std::vector<System*> m_initOrder;
		std::vector<System*> m_initPendingSystems;
		std::vector<System*> m_removePendingSystems;

		std::unordered_map<std::type_index, System*> m_lookup;
		b8 m_exitRequested = false;
	};
}