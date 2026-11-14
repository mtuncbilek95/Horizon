#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/RTTR/Attributes/UnitAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["SpotLight", "Lights"]);
	class SpotLight final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(SpotLight);
	public:
		SpotLight() = default;
		~SpotLight() = default;

		HFIELD();
		Math::Color4f m_color;

		HFIELD(Unit["lm"], Range[0.f, f32_max], Tooltip["Luminous power in lumen (lm). Brightness does not change with cone angle"]);
		f32 m_luminousPower = 800.f;

		HFIELD(Unit["meter"], Range[0.f, f32_max], Tooltip["Influence radius in meters"]);
		f32 m_range = 10.f;

		HFIELD(Unit["deg"], Degrees);
		f32 m_innerConeAngle = Math::DegToRad(20.f);

		HFIELD(Unit["deg"], Degrees);
		f32 m_outerConeAngle = Math::DegToRad(30.f);
	};
}