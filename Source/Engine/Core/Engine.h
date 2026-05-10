#pragma once

#include <Engine/Core/ISystem.h>
#include <Engine/Core/IContext.h>

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Horizon
{
	class Engine
	{
	public:
		Engine() = default;
		~Engine() = default;

		template<typename T, typename... Args, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		void AddSystem(Args&&... args)
		{
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			system->SetEngine(this);
			m_initPendingSystems.push_back(std::move(system));
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		EngineReport RemoveSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_systems.contains(typeIndex))
				return EngineReport(T::TypeName(), "System not found");

			m_finalizePendingSystems.push_back(std::move(m_systems[typeIndex]));
			m_systems.erase(typeIndex);

			return EngineReport();
		}

		template<typename T, typename... Args, typename = std::enable_if_t<std::derived_from<T, IContext>>>
		void AddContext(Args&&... args)
		{
			auto ctx = std::make_unique<T>(std::forward<Args>(args)...);
			ctx->SetEngine(this);
			m_initPendingContexts.push_back(std::move(ctx));
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		EngineReport RemoveContext()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_contexts.contains(typeIndex))
				return EngineReport(T::TypeName(), "System not found");

			m_finalizePendingContexts.push_back(std::move(m_contexts[typeIndex]));
			m_contexts.erase(typeIndex);

			return EngineReport();
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		T& GetSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_systems.contains(typeIndex))
				throw std::runtime_error("System not found");

			return *static_cast<T*>(m_systems[typeIndex].get());
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		bool HasSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			return m_systems.contains(typeIndex);
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, IContext>>>
		T& GetContext()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_contexts.contains(typeIndex))
				throw std::runtime_error("Context not found");

			return *static_cast<T*>(m_contexts[typeIndex].get());
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, IContext>>>
		bool HasContext()
		{
			auto typeIndex = std::type_index(typeid(T));
			return m_contexts.contains(typeIndex);
		}

		void Run();
		void RequestExit(std::string_view reason = "System requested");

	private:
		std::vector<std::unique_ptr<ISystem>> m_initPendingSystems;
		std::vector<std::unique_ptr<ISystem>> m_finalizePendingSystems;
		std::unordered_map<std::type_index, std::unique_ptr<ISystem>> m_systems;

		std::vector<std::unique_ptr<IContext>> m_initPendingContexts;
		std::vector<std::unique_ptr<IContext>> m_finalizePendingContexts;
		std::unordered_map<std::type_index, std::unique_ptr<IContext>> m_contexts;

		b8 m_exitRequested = false;
	};
}