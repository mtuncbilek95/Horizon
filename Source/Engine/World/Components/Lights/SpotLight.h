#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["SpotLight", "Rendering"]);
	class SpotLight final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(SpotLight);
	public:
		SpotLight() = default;
		~SpotLight() = default;

		HFIELD();
		Math::Color4f m_color;

		HFIELD(Range[0.f, f32_max]);
		f32 m_intensity = 1.f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_range = 10.f;

		HFIELD(Degrees);
		f32 m_innerConeAngle = Math::DegToRad(20.f);

		HFIELD(Degrees);
		f32 m_outerConeAngle = Math::DegToRad(30.f);
	};
}