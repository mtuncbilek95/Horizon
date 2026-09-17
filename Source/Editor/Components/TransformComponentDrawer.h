#pragma once

#include <Editor/Components/ComponentDrawer.h>
#include <Engine/World/Components/TransformComponent.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT TransformComponentDrawer : public ComponentDrawer
	{
		HORIZON_TYPE_REFLECT(TransformComponentDrawer);
	public:
		TransformComponentDrawer() = default;
		~TransformComponentDrawer() = default;

		void OnRender() final;

		Engine::ComponentTypeId GetComponentId() const { return Reflect::TypeOf<Engine::TransformComponent>(); }
	};
}
