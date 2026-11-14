#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/UnitAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Scalar.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["LightStressTest", "Benchmark"]);
	class LightStressTest final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(LightStressTest);
	public:
		LightStressTest() = default;
		~LightStressTest() = default;

		HFIELD();
		u32 m_lightInstanceCount = 100;

		HFIELD(Unit["meter"], Range[0.f, f32_max]);
		f32 m_innerRadius = 0.3f;

		HFIELD(Unit["meter"], Range[0.f, f32_max]);
		f32 m_outerRadius = 36.3f;

		HFIELD(Unit["deg/s"], Degrees, Range[0.f, f32_max]);
		f32 m_minSpeed = Math::DegToRad(11.5f);

		HFIELD(Unit["deg/s"], Degrees, Range[0.f, f32_max]);
		f32 m_maxSpeed = Math::DegToRad(57.5f);

		HFIELD(Unit["meter"]);
		f32 m_minHeight = 0.5f;

		HFIELD(Unit["meter"]);
		f32 m_maxHeight = 1.7f;

		HFIELD(Unit["meter"], Range[0.f, f32_max]);
		f32 m_bobAmplitude = 0.3f;
	};
}