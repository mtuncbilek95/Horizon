#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/World/ECS/Definitions.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["TargetFollower", "Camera"]);
	class TargetFollower final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(TargetFollower);
	public:
		TargetFollower() = default;
		~TargetFollower() = default;

		HFIELD();
		Engine::EntityHandle m_target;

		HFIELD(Tooltip["Point on the target the camera looks at, in target space"]);
		Math::Vec3f m_targetPivot = Math::Vec3f(0.f, 1.55f, 0.f);

		HFIELD();
		f32 m_minPitch = -1.05f;

		HFIELD();
		f32 m_maxPitch = 0.61f;

		HFIELD();
		f32 m_minDistance = 1.5f;

		HFIELD();
		f32 m_maxDistance = 4.0f;

		HFIELD(Degrees, Tooltip["Starting yaw around the pivot"]);
		f32 m_defaultYaw = 0.f;

		HFIELD(Degrees, Tooltip["Starting pitch around the pivot"]);
		f32 m_defaultPitch = Math::DegToRad(-10.f);

		HFIELD(Tooltip["Starting distance from the pivot"]);
		f32 m_defaultDistance = 2.5f;

		HFIELD();
		Math::Vec3f m_shoulderOffset = Math::Vec3f(0.5f, 0.1f, 0.f);

		HFIELD();
		f32 m_positionDamping = 0.05f;

		HFIELD(Tooltip["Half-life in seconds for vertical follow, 0 snaps"]);
		f32 m_verticalDamping = 0.15f;

		HFIELD(Tooltip["Half-life in seconds for look rotation, 0 snaps"]);
		f32 m_rotationDamping = 0.08f;

		HFIELD(Tooltip["Pivot movement below this radius is ignored"]);
		f32 m_deadZone = 0.f;

		HFIELD(Tooltip["Swing behind the target when there is no orbit input"]);
		b8 m_autoAlignYaw = true;

		HFIELD(Tooltip["Half-life in seconds for auto align"]);
		f32 m_autoAlignDamping = 0.3f;
	};
}