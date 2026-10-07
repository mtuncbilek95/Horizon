#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class ReadOnlyAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ReadOnlyAttribute);
	public:
		ReadOnlyAttribute() = default;
		~ReadOnlyAttribute() = default;
	};
}