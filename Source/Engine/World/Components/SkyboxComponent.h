#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Guid.h>

namespace Horizon::Engine
{
	HCLASS(ComponentType["Skybox Component", "Lighting", true]);
	class H_EXPORT SkyboxComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(SkyboxComponent);
	public:
		SkyboxComponent() = default;
		~SkyboxComponent() = default;

		HFIELD();
		Math::Vec3f m_tint;

		HFIELD();
		f32 m_intensity = 1.f;
	};
}