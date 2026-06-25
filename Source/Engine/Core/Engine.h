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

		template<typename TSubsystem, typename... Args>
			requires std::is_base_of_v<Subsystem, TSubsystem>
		TSubsystem& AddSubsystem(Args&&... args)
		{
			std::type_index key = std::type_index(typeid(TSubsystem));

			auto it = m_lookup.find(key);
			if (it != m_lookup.end())
			{
				Terminal::Warn("Engine", "{} has already been registered.", key.name());
				return *static_cast<TSubsystem*>(it->second);
			}

			auto* system = Allocator::Create<TSubsystem>(CurrLoc(), std::forward<Args>(args)...);

			m_lookup.emplace(key, system);
			m_initPendingSystems.push_back(system);

			return *system;
		}

		template<typename TSubsystem>
		void RemoveSubsystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TSubsystem)));
			if (it == m_lookup.end())
			{
				Terminal::Warn("Engine", "{} is not registered.", typeid(TSubsystem).name());
				return;
			}

			m_removePendingSystems.push_back(it->second);
		}

		template<typename TModule>
		TModule& GetSubsystem()
		{
			auto it = m_lookup.find(std::type_index(typeid(TModule)));
			Terminal::Assert(it != m_lookup.end(), "Engine", "Subsystem not found");

			return *static_cast<TModule*>(it->second);
		}

		template<typename TModule>
		TModule* TryGetSubsystem()
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

		std::vector<Subsystem*> BuildOrder(const std::vector<Subsystem*>& systems, b8 initialize);

	private:
		std::vector<Subsystem*> m_activeSystems;
		std::vector<Subsystem*> m_initPendingSystems;
		std::vector<Subsystem*> m_removePendingSystems;

		std::unordered_map<std::type_index, Subsystem*> m_lookup;
		b8 m_exitRequested = false;
	};
}