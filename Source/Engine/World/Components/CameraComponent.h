#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/AliasAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["CameraComponent", "Location"]);
	class H_EXPORT CameraComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(CameraComponent);
	public:
		CameraComponent() = default;
		~CameraComponent() = default;

		HFIELD();
		f32 m_fov = 70.f;

		HFIELD();
		f32 m_nearPlane = 0.1f;

		HFIELD();
		f32 m_farPlane = 1000.f;

		HFIELD();
		b8 m_isPerspective = true;
	};
}