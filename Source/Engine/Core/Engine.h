#pragma once

#include <Engine/Core/Submodule.h>

#include <Runtime/Containers/StringView.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>

namespace Horizon
{
	class ReflectionModule;

	class Engine final
	{
	public:
		Engine();
		~Engine();

		template<typename TModule, typename... Args>
			requires std::is_base_of_v<Submodule, TModule>
		TModule& AddModule(Args&&... args)
		{
			std::type_index key = std::type_index(typeid(TModule));
			auto it = m_lookup.find(key);

			if (it != m_lookup.end())
			{
				Terminal::Warn("Engine", "{} has already been registered.", key.name());
				return *static_cast<TModule*>(it->second);
			}

			auto* module = Allocator::Create<TModule>(CurrLoc(), std::forward<Args>(args)...);
			TModule& ref = *module;

			m_lookup.emplace(key, module);
			m_modules.push_back(module);

			return ref;
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
		void RequestExit(StringView reason);

	private:
		std::vector<Submodule*> m_modules;
		std::unordered_map<std::type_index, Submodule*> m_lookup;
		b8 m_exitRequested = false;

		ReflectionModule* m_reflectionModule;
	};
}