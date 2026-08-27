#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Guid.h>

namespace Horizon::Engine
{
	HCLASS(ComponentType["Directional Light Component", "Lighting", true]);
	class H_EXPORT DirectionalLightComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(DirectionalLightComponent);
	public:
		DirectionalLightComponent() = default;
		~DirectionalLightComponent() = default;

		HFIELD();
		Math::Vec3f m_direction;

		HFIELD();
		Math::Vec3f m_color;

		HFIELD();
		f32 m_intensity = 1.f;
	};
}