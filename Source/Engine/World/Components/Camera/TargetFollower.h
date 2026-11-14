#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/World/ECS/Definitions.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/RTTR/Attributes/UnitAttribute.h>
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

		HFIELD(Unit["meter"], Tooltip["Point on the target the camera looks at, in target space"]);
		Math::Vec3f m_targetPivot = Math::Vec3f(0.f, 1.55f, 0.f);

		HFIELD(Unit["deg"], Degrees);
		f32 m_minPitch = Math::DegToRad(-60.f);

		HFIELD(Unit["deg"], Degrees);
		f32 m_maxPitch = Math::DegToRad(35.f);

		HFIELD(Unit["meter"]);
		f32 m_minDistance = 1.5f;

		HFIELD(Unit["meter"]);
		f32 m_maxDistance = 4.0f;

		HFIELD(Unit["deg"], Degrees, Tooltip["Starting yaw around the pivot"]);
		f32 m_defaultYaw = 0.f;

		HFIELD(Unit["deg"], Degrees, Tooltip["Starting pitch around the pivot"]);
		f32 m_defaultPitch = Math::DegToRad(-10.f);

		HFIELD(Unit["meter"], Tooltip["Starting distance from the pivot"]);
		f32 m_defaultDistance = 2.5f;

		HFIELD(Unit["meter"]);
		Math::Vec3f m_shoulderOffset = Math::Vec3f(0.5f, 0.1f, 0.f);

		HFIELD(Unit["s"]);
		f32 m_positionDamping = 0.05f;

		HFIELD(Unit["s"], Tooltip["Half-life in seconds for vertical follow, 0 snaps"]);
		f32 m_verticalDamping = 0.15f;

		HFIELD(Unit["s"], Tooltip["Half-life in seconds for look rotation, 0 snaps"]);
		f32 m_rotationDamping = 0.08f;

		HFIELD(Unit["meter"], Tooltip["Pivot movement below this radius is ignored"]);
		f32 m_deadZone = 0.f;

		HFIELD(Tooltip["Swing behind the target when there is no orbit input"]);
		b8 m_autoAlignYaw = true;

		HFIELD(Unit["s"], Tooltip["Half-life in seconds for auto align"]);
		f32 m_autoAlignDamping = 0.3f;
	};
}