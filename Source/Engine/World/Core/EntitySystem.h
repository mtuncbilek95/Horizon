#pragma once

#include <Engine/World/Core/IEntitySystem.h>
#include <Engine/World/EntityComponentSystem.h>

namespace Horizon
{
	template<typename T>
	class EntitySystem : public IEntitySystem
	{
		friend class EntityComponentSystem;

	public:
		EntitySystem() = default;
		virtual ~EntitySystem() = default;

		EntityComponentSystem* GetECS() const { return m_mainECS; }
		Engine* GetEngine() const { return m_engine; }

		void SetECS(EntityComponentSystem* pECS) { m_mainECS = pECS; }
		void SetEngine(Engine* pEngine) { m_engine; }

		std::string GetObjectType() const final { return TypeName<T>(); }
		std::type_index GetObjectIndex() const final { return std::type_index(typeid(T)); }

		/*template<typename U, typename = std::enable_if_t<std::derived_from<U, IEntitySystem>>>
		U& RequestSystem()
		{
			if (!m_mainECS)
				throw std::runtime_error("Entity System is not attached to ECS");

			return m_mainECS->GetEntitySystem<U>();
		}*/

	private:
		EntityComponentSystem* m_mainECS;
		Engine* m_engine;
	};
}