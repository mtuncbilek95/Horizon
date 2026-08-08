#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT HideInInspectorAttribute : public Reflect::Attribute
	{
	public:
		HideInInspectorAttribute() = default;
		~HideInInspectorAttribute() = default;

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<HideInInspectorAttribute>(); }
	};
}