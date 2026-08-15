#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT HideInInspectorAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(HideInInspectorAttribute);
	public:
		HideInInspectorAttribute() = default;
		~HideInInspectorAttribute() = default;
	};
}