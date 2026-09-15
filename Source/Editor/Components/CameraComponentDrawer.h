#pragma once

#include <Editor/Components/ComponentDrawer.h>
#include <Engine/World/Components/CameraComponent.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT CameraComponentDrawer : public ComponentDrawer
	{
		HORIZON_TYPE_REFLECT(CameraComponentDrawer);
	public:
		CameraComponentDrawer() = default;
		~CameraComponentDrawer() = default;

		void OnRender();

		Engine::ComponentTypeId GetComponentId() const { return Reflect::TypeOf<Engine::CameraComponent>(); }
	};
}