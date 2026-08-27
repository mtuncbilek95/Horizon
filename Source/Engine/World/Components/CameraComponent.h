#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Guid.h>

namespace Horizon::Engine
{
	HCLASS(ComponentType["Camera Component", "Visual", true]);
	class H_EXPORT CameraComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(CameraComponent);
	public:
		CameraComponent() = default;
		~CameraComponent() = default;

		HFIELD();
		f32 m_fieldOfView = 60.f;

		HFIELD();
		f32 m_nearPlane = 0.1f;

		HFIELD();
		f32 m_farPlane = 1000.f;
	};
}