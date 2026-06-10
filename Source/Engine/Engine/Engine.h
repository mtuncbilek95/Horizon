#pragma once

#include <Runtime/Log/Log.h>
#include <Engine/Engine/IModule.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <string_view>
#include <cassert>

namespace Horizon
{
	class Engine final
	{
	public:
		template<typename TModule, typename... Args>
			requires std::is_base_of_v<IModule, TModule>
		TModule& AddModule(Args&&... args)
		{
			std::type_index key = std::type_index(typeid(TModule));
			auto it = m_lookup.find(key);

			if (it != m_lookup.end())
			{
				MainLog::Warn("{} has already been registered.", key.name());
				return *static_cast<TModule*>(it->second);
			}

			auto module = std::make_unique<TModule>(std::forward<Args>(args)...);
			TModule& ref = *module;

			m_lookup.emplace(key, module.get());
			m_modules.push_back(std::move(module));

			return ref;
		}

		template<typename TModule>
		TModule& GetModule()
		{
			auto it = m_lookup.find(std::type_index(typeid(TModule)));
			assert(it != m_lookup.end() && "Module not found");

			return *static_cast<TModule*>(it->second);
		}

		template<typename TModule>
		TModule* TryGetModule()
		{
			auto it = m_lookup.find(std::type_index(typeid(TModule)));
			return it == m_lookup.end() ? nullptr : static_cast<TModule*>(it->second);
		}

		void Run();
		void RequestExit(std::string_view reason) 
		{
			m_exitRequested = true;
			MainLog::Info("{}", reason);
		}

	private:
		std::vector<std::unique_ptr<IModule>> m_modules;
		std::unordered_map<std::type_index, IModule*> m_lookup;
		b8 m_exitRequested = false;
	};
}