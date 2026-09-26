#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/AliasAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Mat4f.h>

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
		Math::Vec2f m_targetScreen = { 0.f, 0.f };

		Math::Mat4f m_viewProjection;
		Math::Mat4f m_view;
		Math::Mat4f m_projection;
		Math::Vec3f m_worldPosition;
	};
}