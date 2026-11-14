#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["AreaLight", "Lights"]);
	class AreaLight final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(AreaLight);
	public:
		AreaLight() = default;
		~AreaLight() = default;

		HFIELD();
		Math::Color4f m_color;

		HFIELD(Range[0.f, f32_max]);
		f32 m_intensity = 1.f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_width = 1.f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_height = 1.f;

		HFIELD();
		b8 m_twoSided = false;
	};
}