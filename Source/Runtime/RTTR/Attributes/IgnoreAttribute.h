#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class IgnoreAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(IgnoreAttribute);
	public:
		IgnoreAttribute() = default;
		~IgnoreAttribute() = default;
	};
}