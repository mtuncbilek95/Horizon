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

		HFIELD(Unit["lux"], Range[0.f, f32_max], Tooltip["Illuminance in lux (lx). Clear noon sun ~100000, overcast ~10000, sunset ~400"]);
		f32 m_illuminance = 100000.f;
	};
}