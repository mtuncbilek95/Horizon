#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class SliderAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(SliderAttribute);
	public:
		SliderAttribute() = default;
		~SliderAttribute() = default;
	};
}