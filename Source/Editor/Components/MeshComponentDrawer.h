#pragma once

#include <Editor/Components/ComponentDrawer.h>
#include <Engine/World/Components/MeshComponent.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT MeshComponentDrawer : public ComponentDrawer
	{
		HORIZON_TYPE_REFLECT(MeshComponentDrawer);
	public:
		MeshComponentDrawer() = default;
		~MeshComponentDrawer() = default;

		void OnRender();

		Engine::ComponentTypeId GetComponentId() const { return Reflect::TypeOf<Engine::MeshComponent>(); }

	private:
		std::string m_currentLabel;
	};
}