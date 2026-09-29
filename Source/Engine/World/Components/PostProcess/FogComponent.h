#pragma once

#include <Engine/Rendering/FogMode.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/SliderAttribute.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["FogComponent", "Postprocess"]);
	class H_EXPORT FogComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(FogComponent);
	public:
		FogComponent() = default;
		~FogComponent() = default;

		HFIELD();
		FogMode m_mode = FogMode::Linear;

		HFIELD();
		Math::Color4f m_color = Math::Color4f::White();

		HFIELD();
		f32 m_density = 0.f;

		HFIELD(Range[0, f32_max]);
		f32 m_startDistance = 0.f;

		HFIELD(Range[0, f32_max]);
		f32 m_endDistance = 0.f;

		HFIELD(Slider, Range[0, 1]);
		f32 m_maxOpacity = 1.0f;
	};
}