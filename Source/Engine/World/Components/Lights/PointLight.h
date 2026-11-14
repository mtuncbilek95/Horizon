#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/RTTR/Attributes/TransientAttribute.h>
#include <Runtime/RTTR/Attributes/UnitAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["PointLight", "Lights"]);
	class PointLight final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(PointLight);
	public:
		PointLight() = default;
		~PointLight() = default;

		HFIELD();
		Math::Color4f m_color;

		HFIELD(Unit["lm"], Range[0.f, f32_max], Tooltip["Luminous power in lumen (lm). 60W incandescent bulb ~800"]);
		f32 m_luminousPower = 800.f;

		HFIELD(Unit["meter"], Range[0.f, f32_max], Tooltip["Influence radius in meters"]);
		f32 m_range = 10.f;

		HFIELD(Transient);
		b8 m_drawDebugObject = false;
	};
}