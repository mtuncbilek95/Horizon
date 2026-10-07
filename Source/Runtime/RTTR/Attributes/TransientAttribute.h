#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class RUNTIME_API TransientAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(TransientAttribute);
	public:
		TransientAttribute() = default;
		~TransientAttribute() = default;
	};
}