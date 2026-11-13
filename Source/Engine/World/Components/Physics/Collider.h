#pragma once

#include <Engine/Physics/ColliderShape.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["Collider", "Physics"]);
	class Collider final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(Collider);
	public:
		Collider() = default;
		~Collider() = default;

		HFIELD();
		Engine::ColliderShape m_shape = Engine::ColliderShape::Box;

		HFIELD();
		Math::Vec3f m_halfExtents = { 0.5f, 0.5f, 0.5f };

		HFIELD(Range[0.01f, f32_max]);
		f32 m_radius = 0.5f;

		HFIELD(Range[0.01f, f32_max]);
		f32 m_halfHeight = 0.5f;

		HFIELD();
		Math::Vec3f m_offset = { 0.f, 0.f, 0.f };
	};
}