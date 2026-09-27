#pragma once

#include <Editor/Components/ComponentDrawer.h>
#include <Engine/World/Components/FogComponent.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT FogComponentDrawer : public ComponentDrawer
	{
		HORIZON_TYPE_REFLECT(FogComponentDrawer);
	public:
		FogComponentDrawer() = default;
		~FogComponentDrawer() = default;

		void OnRender();

		Engine::ComponentTypeId GetComponentId() const { return Reflect::TypeOf<Engine::FogComponent>(); }

	private:
	};
}