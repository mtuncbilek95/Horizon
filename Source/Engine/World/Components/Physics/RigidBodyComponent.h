#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/Physics/PhysicsMotion.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["RigidBodyComponent", "Physics"]);
	class H_EXPORT RigidBodyComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(RigidBodyComponent);
	public:
		RigidBodyComponent() = default;
		~RigidBodyComponent() = default;

		HFIELD();
		PhysicsMotion m_motion = PhysicsMotion::Static;

		HFIELD();
		Math::Vec3f m_halfExtents = Math::Vec3f::Zero();

		HFIELD(Range[0, f32_max]);
		f32 m_mass = 0;

		u32 m_bodyId = kInvalid32;
	};
}