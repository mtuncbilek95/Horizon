#pragma once

#include <Engine/Engine/Engine.h>
#include <Engine/ECS/IECS.h>
#include <Engine/System/SystemReport.h>
#include <Engine/Asset/Scene/SceneAsset.h>

#include <entt/entt.hpp>

namespace Horizon
{
	template<typename T>
	class ECS : public IECS
	{
		friend class Engine;
	public:
		ECS() = default;
		virtual ~ECS() = default;

		Engine* GetEngine() const { return m_mainEngine; }
		void SetEngine(Engine* pEngine) { m_mainEngine = pEngine; }

		entt::registry& GetScene() const { return *m_currentRegistry; }
		void SetScene(entt::registry* scene) { m_currentRegistry = scene; }
		entt::registry& GetRegistry() const
		{
			if (!m_currentRegistry)
				throw std::runtime_error("ECS is not attached to a scene");

			return *m_currentRegistry;
		}

		std::string_view GetObjectType() const final { return TypeName<T>(); }
		std::type_index GetObjectIndex() const final { return std::type_index(typeid(T)); }

		template<typename U, typename = std::enable_if_t<std::derived_from<U, ISystem>>>
		U& RequestSystem()
		{
			if (!m_mainEngine)
				throw std::runtime_error("System is not attached to an engine");

			return m_mainEngine->GetSystem<U>();
		}

	private:
		Engine* m_mainEngine = nullptr;
		entt::registry* m_currentRegistry = nullptr;
	};
}