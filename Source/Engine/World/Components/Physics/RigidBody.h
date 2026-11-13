#pragma once

#include <Engine/Physics/PhysicsMotion.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["RigidBody", "Physics"]);
	class RigidBody final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(RigidBody);
	public:
		RigidBody() = default;
		~RigidBody() = default;

		HFIELD();
		Engine::PhysicsMotion m_motion = Engine::PhysicsMotion::Static;

		HFIELD(Range[0.001f, f32_max]);
		f32 m_mass = 10.f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_friction = 1.f;

		HFIELD(Range[0.f, 1.f]);
		f32 m_restitution = 0.f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_linearDamping = 0.f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_angularDamping = 0.f;

		HFIELD();
		f32 m_gravityFactor = 1.f;

		HFIELD();
		b8 m_continuousCollision = false;

		HFIELD();
		b8 m_isTrigger = false;
	};
}