#pragma once

#include <Editor/Components/ComponentDrawer.h>
#include <Engine/World/Components/Physics/RigidBodyComponent.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT RigidBodyComponentDrawer : public ComponentDrawer
	{
		HORIZON_TYPE_REFLECT(RigidBodyComponentDrawer);
	public:
		RigidBodyComponentDrawer() = default;
		~RigidBodyComponentDrawer() = default;

		void OnRender();

		Engine::ComponentTypeId GetComponentId() const { return Reflect::TypeOf<Engine::RigidBodyComponent>(); }

	private:
		std::string m_currentLabel;
	};
}