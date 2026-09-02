#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["TransformComponent", "Location"]);
	class H_EXPORT TransformComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(TransformComponent);
	public:
		TransformComponent() = default;
		~TransformComponent() = default;

		HFIELD();
		Math::Vec3f m_position;

		HFIELD();
		Math::Quat m_rotation;

		HFIELD();
		Math::Vec3f m_scale;
	};
}