#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/RTTR/Attributes/UnitAttribute.h>
#include <Runtime/Math/Scalar.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["PerspectiveCamera", "Camera"]);
	class PerspectiveCamera final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(PerspectiveCamera);
	public:
		PerspectiveCamera() = default;
		~PerspectiveCamera() = default;

		HFIELD(Unit["deg"], Degrees, Tooltip["Represents in degrees"]);
		f32 m_fov = Math::DegToRad(70.f);

		HFIELD(Unit["meter"]);
		f32 m_nearPlane = 0.1f;

		HFIELD(Unit["meter"], Tooltip["Cluster depth: light clusters are sliced up to this distance. Not a clip plane, projection is infinite reverse-Z"]);
		f32 m_farPlane = 1000.f;

		HFIELD(Tooltip["Highest priority non-editor camera becomes the game view"]);
		i32 m_priority = 0;
	};
}