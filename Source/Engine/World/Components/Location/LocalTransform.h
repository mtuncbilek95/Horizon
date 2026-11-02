#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["LocalTransform", "Location"]);
	class LocalTransform final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(LocalTransform);
	public:
		LocalTransform() = default;
		~LocalTransform() = default;

		HFIELD();
		Math::Vec3f m_position;

		HFIELD(Degrees, Tooltip["Represents as degrees"]);
		Math::Vec3f m_rotation;

		HFIELD();
		Math::Vec3f m_scale = { 1.f, 1.f, 1.f };
	};
}