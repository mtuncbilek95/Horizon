#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Engine
{
	HCLASS(ComponentType["Transform Component", "Location", true]);
	class H_EXPORT TransformComponent : public ComponentObject 
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