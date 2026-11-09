#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class DynamicUsageAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(DynamicUsageAttribute);
	public:
		DynamicUsageAttribute() = default;
		~DynamicUsageAttribute() = default;
	};
}