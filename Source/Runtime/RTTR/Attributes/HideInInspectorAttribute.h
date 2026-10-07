#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class HideInInspectorAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(HideInInspectorAttribute);
	public:
		HideInInspectorAttribute() = default;
		~HideInInspectorAttribute() = default;
	};
}