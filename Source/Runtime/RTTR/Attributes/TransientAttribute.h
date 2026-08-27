#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class H_EXPORT TransientAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(TransientAttribute);
	public:
		TransientAttribute() = default;
		~TransientAttribute() = default;
	};
}