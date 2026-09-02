#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT ComponentObject : public Reflect::Base
	{
	public:
		ComponentObject() = default;
		virtual ~ComponentObject() = default;

		ComponentObject(const ComponentObject&) = default;
		ComponentObject& operator=(const ComponentObject&) = default;

		ComponentObject(ComponentObject&&) noexcept = default;
		ComponentObject& operator=(ComponentObject&&) noexcept = default;
	};
}