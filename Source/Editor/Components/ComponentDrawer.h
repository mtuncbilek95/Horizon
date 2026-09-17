#pragma once

#include <Engine/World/ECS/Definitions.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Engine;
	class Scene;
}

namespace Horizon::Editor
{
	class H_EXPORT ComponentDrawer : public Reflect::Base
	{
		friend class InspectorView;
	public:
		virtual ~ComponentDrawer() override = default;

		virtual void OnRender() = 0;
		virtual Engine::ComponentTypeId GetComponentId() const = 0;

		Engine::Engine* GetEngine() const { return m_engine; }

		template<typename T>
		T* GetComponent() const { return (T*)GetComponent(); }
		Engine::ComponentObject* GetComponent() const { return m_component; }

	private:
		Engine::Engine* m_engine;
		Engine::ComponentObject* m_component;
	};
}