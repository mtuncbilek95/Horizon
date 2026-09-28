#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["SpinTestComponent", "Script"]);
	class H_EXPORT SpinTestComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(SpinTestComponent);
	public:
		SpinTestComponent() = default;
		~SpinTestComponent() = default;

		HFIELD();
		f32 m_speed = 15.f;

		HFIELD();
		Math::Vec3f m_axis = { 0.f, 1.f, 0.f };
	};
}