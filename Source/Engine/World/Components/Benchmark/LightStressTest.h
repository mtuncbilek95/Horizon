#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["LightStressTest", "Rendering"]);
	class LightStressTest final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(LightStressTest);
	public:
		LightStressTest() = default;
		~LightStressTest() = default;

		HFIELD();
		u32 m_lightInstanceCount = 100;

		HFIELD(Range[0.f, f32_max]);
		f32 m_innerRadius = 0.3f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_outerRadius = 36.3f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_minSpeed = 0.2f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_maxSpeed = 1.f;

		HFIELD();
		f32 m_minHeight = 0.5f;

		HFIELD();
		f32 m_maxHeight = 1.7f;

		HFIELD(Range[0.f, f32_max]);
		f32 m_bobAmplitude = 0.3f;
	};
}