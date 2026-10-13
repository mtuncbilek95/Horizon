#pragma once

#include <Engine/World/ECS/Definitions.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Engine;
	class ScriptSystem;

	HCLASS();
	class ENGINE_API GameScript : public Reflect::Base
	{
		friend class ScriptSystem;
	public:
		GameScript() = default;
		virtual ~GameScript() = default;

		virtual void OnStart() = 0;
		virtual void OnUpdate(f32 deltaTime) = 0;
		virtual void OnStop() = 0;

	protected:
		EntityHandle GetEntity() const { return m_entity; }
		Scene* GetScene() const { return m_scene; }
		Engine* GetEngine() const { return m_engine; }

		template<typename T>
		T* GetComponent(EntityHandle entt) const
		{
			return m_scene->FindComponent<T>(entt);
		}

	private:
		EntityHandle m_entity;
		Scene* m_scene = nullptr;
		Engine* m_engine = nullptr;
		b8 m_started = false;
	};
}