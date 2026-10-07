#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class DegreesAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(DegreesAttribute);
	public:
		DegreesAttribute() = default;
		~DegreesAttribute() = default;
	};
}