#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/RTTR/Attributes/UnitAttribute.h>
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

		HFIELD(Unit["lm"], Range[0.f, f32_max], Tooltip["Luminous power in lumen (lm), spread over the emitting surface"]);
		f32 m_luminousPower = 800.f;

		HFIELD(Unit["meter"], Range[0.f, f32_max], Tooltip["Width in meters"]);
		f32 m_width = 1.f;

		HFIELD(Unit["meter"], Range[0.f, f32_max], Tooltip["Height in meters"]);
		f32 m_height = 1.f;

		HFIELD();
		b8 m_twoSided = false;
	};
}