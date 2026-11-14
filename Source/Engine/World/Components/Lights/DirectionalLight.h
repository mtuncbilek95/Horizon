#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["DirectionalLight", "Lights"]);
	class DirectionalLight final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(DirectionalLight);
	public:
		DirectionalLight() = default;
		~DirectionalLight() = default;

		HFIELD();
		Math::Color4f m_color;

		HFIELD(Range[-1.f, 1.f]);
		Math::Vec3f m_direction;

		HFIELD(Range[0.f, f32_max]);
		f32 m_intensity = 1.0f;
	};
}